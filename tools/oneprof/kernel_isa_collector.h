//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#ifndef PTI_TOOLS_ONEPROF_KERNEL_ISA_COLLECTOR_H_
#define PTI_TOOLS_ONEPROF_KERNEL_ISA_COLLECTOR_H_

#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include <level_zero/layers/zel_tracing_api.h>

#include "result_storage.h"
#include "ze_utils.h"

// Records the GPU base address and the ISA of every Level Zero kernel the
// application creates, for EU stall sampling (--stall-sampling): the sampled
// instruction pointers are mapped back to kernels and instructions with them.
class KernelIsaCollector {
 public:
  static KernelIsaCollector* Create(ze_driver_handle_t driver) {
    PTI_ASSERT(driver != nullptr);

    BaseAddressFunction get_base_address = nullptr;
    ze_result_t status = zeDriverGetExtensionFunctionAddress(
        driver, "zexKernelGetBaseAddress",
        reinterpret_cast<void**>(&get_base_address));
    if (status != ZE_RESULT_SUCCESS || get_base_address == nullptr) {
      std::cerr << "[WARNING] zexKernelGetBaseAddress is not supported " <<
        "by the driver: EU stall samples can't be mapped to kernels" <<
        std::endl;
      return nullptr;
    }

    KernelIsaCollector* collector = new KernelIsaCollector(get_base_address);
    PTI_ASSERT(collector != nullptr);

    zel_tracer_desc_t tracer_desc = {
        ZEL_STRUCTURE_TYPE_TRACER_EXP_DESC, nullptr, collector};
    zel_tracer_handle_t tracer = nullptr;
    status = zelTracerCreate(&tracer_desc, &tracer);
    if (status != ZE_RESULT_SUCCESS) {
      std::cerr << "[WARNING] Unable to create Level Zero tracer" << std::endl;
      delete collector;
      return nullptr;
    }

    collector->EnableTracing(tracer);
    return collector;
  }

  ~KernelIsaCollector() {
    if (tracer_ != nullptr) {
      ze_result_t status = zelTracerDestroy(tracer_);
      PTI_ASSERT(status == ZE_RESULT_SUCCESS);
    }
  }

  void DisableTracing() {
    PTI_ASSERT(tracer_ != nullptr);
    ze_result_t status = zelTracerSetEnabled(tracer_, false);
    PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  }

  std::vector<KernelIsa> GetKernelIsaList() {
    const std::lock_guard<std::mutex> lock(lock_);
    return kernel_isa_list_;
  }

  KernelIsaCollector(const KernelIsaCollector& copy) = delete;
  KernelIsaCollector& operator=(const KernelIsaCollector& copy) = delete;

 private:
  typedef ze_result_t (*BaseAddressFunction)(ze_kernel_handle_t, uint64_t*);

  explicit KernelIsaCollector(BaseAddressFunction get_base_address)
      : get_base_address_(get_base_address) {}

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

    uint64_t base_address = 0;
    ze_result_t status = get_base_address_(kernel, &base_address);
    if (status != ZE_RESULT_SUCCESS || base_address == 0) {
      return;
    }

    {
      const std::lock_guard<std::mutex> lock(lock_);
      for (const auto& kernel_isa : kernel_isa_list_) {
        if (kernel_isa.base_address == base_address) {
          return; // The same kernel was created again
        }
      }
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

    const std::lock_guard<std::mutex> lock(lock_);
    kernel_isa_list_.push_back(kernel_isa);
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

  std::mutex lock_;
  std::vector<KernelIsa> kernel_isa_list_;
};

#endif // PTI_TOOLS_ONEPROF_KERNEL_ISA_COLLECTOR_H_
