//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#ifndef PTI_TOOLS_ONEPROF_KERNEL_ISA_COLLECTOR_H_
#define PTI_TOOLS_ONEPROF_KERNEL_ISA_COLLECTOR_H_

#include <cstdio>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <level_zero/layers/zel_tracing_api.h>

#include "cl_api_tracer.h"
#include "cl_utils.h"
#include "result_storage.h"
#include "ze_utils.h"

// Intel OpenCL kernel queries (NEO): the kernel's GPU address and ISA, the
// same values as zexKernelGetBaseAddress and zeKernelGetBinaryExp give.
#ifndef CL_KERNEL_BINARY_PROGRAM_INTEL
#define CL_KERNEL_BINARY_PROGRAM_INTEL 0x407D
#endif
#ifndef CL_KERNEL_BINARY_GPU_ADDRESS_INTEL
#define CL_KERNEL_BINARY_GPU_ADDRESS_INTEL 0x10010
#endif

// Records the GPU base address and the ISA of every Level Zero kernel the
// application creates and of every OpenCL kernel it enqueues on the target
// device, for EU stall sampling (--stall-sampling): the sampled instruction
// pointers are mapped back to kernels and instructions with them.
class KernelIsaCollector {
 public:
  // cl_device may be nullptr (no OpenCL device for the target GPU).
  static KernelIsaCollector* Create(ze_driver_handle_t driver,
                                    cl_device_id cl_device) {
    PTI_ASSERT(driver != nullptr);

    KernelIsaCollector* collector = new KernelIsaCollector();
    PTI_ASSERT(collector != nullptr);

    ze_result_t status = zeDriverGetExtensionFunctionAddress(
        driver, "zexKernelGetBaseAddress",
        reinterpret_cast<void**>(&collector->get_base_address_));
    if (status != ZE_RESULT_SUCCESS ||
        collector->get_base_address_ == nullptr) {
      std::cerr << "[WARNING] zexKernelGetBaseAddress is not supported " <<
        "by the driver: EU stall samples can't be mapped to Level Zero " <<
        "kernels" << std::endl;
      collector->get_base_address_ = nullptr;
    } else {
      zel_tracer_desc_t tracer_desc = {
          ZEL_STRUCTURE_TYPE_TRACER_EXP_DESC, nullptr, collector};
      zel_tracer_handle_t tracer = nullptr;
      status = zelTracerCreate(&tracer_desc, &tracer);
      if (status != ZE_RESULT_SUCCESS) {
        std::cerr << "[WARNING] Unable to create Level Zero tracer" <<
          std::endl;
      } else {
        collector->EnableTracing(tracer);
      }
    }

    if (cl_device != nullptr && !IsClGpuAddressValid(cl_device)) {
      std::cerr << "[INFO] OpenCL kernels are not mapped to EU stall " <<
        "samples: this needs compute runtime 25.48 or newer" << std::endl;
    } else if (cl_device != nullptr) {
      ClApiTracer* cl_tracer = new ClApiTracer(cl_device, OnClCall, collector);
      PTI_ASSERT(cl_tracer != nullptr);
      if (!cl_tracer->IsValid() ||
          !cl_tracer->SetTracingFunction(CL_FUNCTION_clEnqueueNDRangeKernel) ||
          !cl_tracer->SetTracingFunction(CL_FUNCTION_clEnqueueTask) ||
          !cl_tracer->Enable()) {
        std::cerr << "[WARNING] Unable to trace OpenCL: EU stall samples " <<
          "can't be mapped to OpenCL kernels" << std::endl;
        delete cl_tracer;
      } else {
        collector->cl_tracer_ = cl_tracer;
      }
    }

    if (collector->tracer_ == nullptr && collector->cl_tracer_ == nullptr) {
      delete collector;
      return nullptr;
    }
    return collector;
  }

  ~KernelIsaCollector() {
    if (tracer_ != nullptr) {
      ze_result_t status = zelTracerDestroy(tracer_);
      PTI_ASSERT(status == ZE_RESULT_SUCCESS);
    }
    if (cl_tracer_ != nullptr) {
      delete cl_tracer_;
    }
  }

  void DisableTracing() {
    if (tracer_ != nullptr) {
      ze_result_t status = zelTracerSetEnabled(tracer_, false);
      PTI_ASSERT(status == ZE_RESULT_SUCCESS);
    }
    if (cl_tracer_ != nullptr) {
      bool disabled = cl_tracer_->Disable();
      PTI_ASSERT(disabled);
    }
  }

  std::vector<KernelIsa> GetKernelIsaList() {
    const std::lock_guard<std::mutex> lock(lock_);
    return kernel_isa_list_;
  }

  KernelIsaCollector(const KernelIsaCollector& copy) = delete;
  KernelIsaCollector& operator=(const KernelIsaCollector& copy) = delete;

 private:
  typedef ze_result_t (*BaseAddressFunction)(ze_kernel_handle_t, uint64_t*);

  KernelIsaCollector() {}

  // Before compute runtime 25.48, CL_KERNEL_BINARY_GPU_ADDRESS_INTEL is the
  // address of the kernel's own allocation, not of the ISA the GPU runs (in
  // the ISA pool): samples would be mapped to no kernel or to the wrong one.
  // The Linux driver version is <year>.<week>.<build>.<hotfix>.
  static bool IsClGpuAddressValid(cl_device_id device) {
    char version[MAX_STR_SIZE] = { 0 };
    cl_int status = clGetDeviceInfo(
        device, CL_DRIVER_VERSION, MAX_STR_SIZE - 1, version, nullptr);
    if (status != CL_SUCCESS) {
      return false;
    }
    unsigned year = 0, week = 0;
    if (sscanf(version, "%u.%u.", &year, &week) != 2) {
      return true;  // Another version scheme: no known issue
    }
    return year > 25 || (year == 25 && week >= 48);
  }

  void EnableTracing(zel_tracer_handle_t tracer) {
    PTI_ASSERT(tracer != nullptr);
    tracer_ = tracer;

    zet_core_callbacks_t prologue_callbacks{};
    zet_core_callbacks_t epilogue_callbacks{};
    epilogue_callbacks.Kernel.pfnCreateCb = OnExitKernelCreate;

    ze_result_t status = zelTracerSetPrologues(tracer_, &prologue_callbacks);
    PTI_ASSERT(status == ZE_RESULT_SUCCESS);
    status = zelTracerSetEpilogues(tracer_, &epilogue_callbacks);
    PTI_ASSERT(status == ZE_RESULT_SUCCESS);
    status = zelTracerSetEnabled(tracer_, true);
    PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  }

  void AddKernel(ze_kernel_handle_t kernel) {
    PTI_ASSERT(kernel != nullptr);

    PTI_ASSERT(get_base_address_ != nullptr);

    uint64_t base_address = 0;
    ze_result_t status = get_base_address_(kernel, &base_address);
    if (status != ZE_RESULT_SUCCESS || base_address == 0 ||
        IsRecorded(base_address)) {
      return;
    }

    KernelIsa kernel_isa{};
    kernel_isa.name = utils::ze::GetKernelName(kernel);
    kernel_isa.base_address = base_address;

    // Without the ISA the kernel is still mapped (up to the next kernel), but
    // there is no disassembly for it.
    size_t size = 0;
    status = zeKernelGetBinaryExp(kernel, &size, nullptr);
    if (status == ZE_RESULT_SUCCESS && size > 0) {
      kernel_isa.binary.resize(size);
      status = zeKernelGetBinaryExp(kernel, &size, kernel_isa.binary.data());
      if (status != ZE_RESULT_SUCCESS) {
        kernel_isa.binary.clear();
      }
    }

    Record(kernel_isa);
  }

  void AddKernel(cl_kernel kernel) {
    PTI_ASSERT(kernel != nullptr);

    uint64_t base_address = 0;
    cl_int status = clGetKernelInfo(
        kernel, CL_KERNEL_BINARY_GPU_ADDRESS_INTEL, sizeof(base_address),
        &base_address, nullptr);
    if (status != CL_SUCCESS || base_address == 0 ||
        IsRecorded(base_address)) {
      return; // Not supported, or the kernel is known (enqueued again)
    }

    KernelIsa kernel_isa{};
    kernel_isa.name = utils::cl::GetKernelName(kernel);
    kernel_isa.base_address = base_address;

    size_t size = 0;
    status = clGetKernelInfo(
        kernel, CL_KERNEL_BINARY_PROGRAM_INTEL, 0, nullptr, &size);
    if (status == CL_SUCCESS && size > 0) {
      kernel_isa.binary.resize(size);
      status = clGetKernelInfo(
          kernel, CL_KERNEL_BINARY_PROGRAM_INTEL, size,
          kernel_isa.binary.data(), nullptr);
      if (status != CL_SUCCESS) {
        kernel_isa.binary.clear();
      }
    }

    Record(kernel_isa);
  }

  bool IsRecorded(uint64_t base_address) {
    const std::lock_guard<std::mutex> lock(lock_);
    return address_set_.count(base_address) > 0;
  }

  void Record(const KernelIsa& kernel_isa) {
    const std::lock_guard<std::mutex> lock(lock_);
    if (address_set_.insert(kernel_isa.base_address).second) {
      kernel_isa_list_.push_back(kernel_isa);
    }
  }

  template <typename T>
  static cl_kernel GetEnqueuedKernel(cl_callback_data* data) {
    const T* params = reinterpret_cast<const T*>(data->functionParams);
    PTI_ASSERT(params != nullptr);
    return *(params->kernel);
  }

  static void OnClCall(ClFunctionId function, cl_callback_data* data,
                       void* user_data) {
    PTI_ASSERT(data != nullptr);
    if (data->site != CL_CALLBACK_SITE_EXIT) {
      return;
    }
    const cl_int* result =
      reinterpret_cast<const cl_int*>(data->functionReturnValue);
    if (result == nullptr || *result != CL_SUCCESS) {
      return;
    }

    cl_kernel kernel = nullptr;
    if (function == CL_FUNCTION_clEnqueueNDRangeKernel) {
      kernel = GetEnqueuedKernel<cl_params_clEnqueueNDRangeKernel>(data);
    } else if (function == CL_FUNCTION_clEnqueueTask) {
      kernel = GetEnqueuedKernel<cl_params_clEnqueueTask>(data);
    }
    if (kernel == nullptr) {
      return;
    }

    KernelIsaCollector* collector =
      reinterpret_cast<KernelIsaCollector*>(user_data);
    PTI_ASSERT(collector != nullptr);
    collector->AddKernel(kernel);
  }

  static void OnExitKernelCreate(
      ze_kernel_create_params_t* params, ze_result_t result,
      void* global_user_data, void** /*instance_user_data*/) {
    if (result != ZE_RESULT_SUCCESS) {
      return;
    }
    KernelIsaCollector* collector =
      reinterpret_cast<KernelIsaCollector*>(global_user_data);
    PTI_ASSERT(collector != nullptr);
    PTI_ASSERT(params != nullptr && params->pphKernel != nullptr);
    if (*(params->pphKernel) != nullptr && **(params->pphKernel) != nullptr) {
      collector->AddKernel(**(params->pphKernel));
    }
  }

 private:
  zel_tracer_handle_t tracer_ = nullptr;
  BaseAddressFunction get_base_address_ = nullptr;
  ClApiTracer* cl_tracer_ = nullptr;

  std::mutex lock_;
  std::set<uint64_t> address_set_;
  std::vector<KernelIsa> kernel_isa_list_;
};

#endif // PTI_TOOLS_ONEPROF_KERNEL_ISA_COLLECTOR_H_
