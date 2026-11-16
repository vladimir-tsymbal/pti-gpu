# Runtime API Tracing for oneAPI Level Zero (Level Zero)
## Overview
Level Zero API tracing allows to intercept standard API calls by injecting user-defined callbacks to function enter and exit points, as well as to read and modify traced function arguments and to read return value.

API tracing is implemented as one of the layers in Level Zero loader.

**Supported Runtimes**:
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

**Supported OS**:
- Linux
- Windows (not validated)

**Supported HW**:
- Tested on Intel(R) Data Center GPU Max 1550 (Xe-HPC, i915 driver) and Intel(R) Arc(TM) B570 Graphics (Xe2, xe driver); other GPUs, including older generations, are not tested

**Needed Headers**:
- [zet_api.h](https://github.com/oneapi-src/level-zero/blob/master/include/zet_api.h)
- [zel_tracing_api.h](https://github.com/oneapi-src/level-zero/blob/master/include/layers/zel_tracing_api.h)
- [zel_tracing_register_cb.h](https://github.com/oneapi-src/level-zero/blob/master/include/layers/zel_tracing_register_cb.h) (APIs newer than Level Zero 1.0)

**Needed Libraries**:
- oneAPI Level Zero [libraries](https://github.com/intel/compute-runtime)

## How To Use
1. Before any work with Level Zero one must initialize its core and tools API:
```cpp
ze_result_t status = ZE_RESULT_SUCCESS;
// zeInitDrivers() needs Level Zero 1.10+ headers and loader
ze_init_driver_type_desc_t driver_desc = {
    ZE_STRUCTURE_TYPE_INIT_DRIVER_TYPE_DESC, nullptr,
    ZE_INIT_DRIVER_TYPE_FLAG_GPU};
uint32_t driver_count = 0;
status = zeInitDrivers(&driver_count, nullptr, &driver_desc);
if (status != ZE_RESULT_SUCCESS || driver_count == 0) {
  // Older loader or driver: fall back to the legacy initialization
  status = zeInit(ZE_INIT_FLAG_GPU_ONLY);
}
assert(status == ZE_RESULT_SUCCESS);
```
The samples and tools do the same in `utils::ze::InitGpuDrivers()` ([ze_utils.h](../../utils/ze_utils.h)). Drivers and devices are then listed with `zeDriverGet` and `zeDeviceGet`.
2. For each traced function on need to define the callbacks that will be called along with the traced API on enter and on exit, e.g. for `zeCommandListAppendLaunchKernel`:
```cpp
void OnEnterCommandListAppendLaunchKernel(
    ze_command_list_append_launch_kernel_params_t* params,
    ze_result_t result, void* global_user_data, void** instance_user_data) {
  std::cout << "Function zeCommandListAppendLaunchKernel is called on enter" <<
    std::endl;
}

void OnExitCommandListAppendLaunchKernel(
    ze_command_list_append_launch_kernel_params_t* params,
    ze_result_t result, void* global_user_data, void** instance_user_data) {
  std::cout << "Function zeCommandListAppendLaunchKernel is called on exit" <<
    std::endl;
}
```
3. Create a tracer handle:
```cpp
  zel_tracer_desc_t tracer_desc = {
      ZEL_STRUCTURE_TYPE_TRACER_DESC, nullptr,
      nullptr /* global user data */};

  zel_tracer_handle_t tracer = nullptr;
  status = zelTracerCreate(&tracer_desc, &tracer);
  assert(status == ZE_RESULT_SUCCESS);
```
4. Set callbacks to Level Zero API functions that should be traced, e.g.:
```cpp
  zet_core_callbacks_t prologue_callbacks = {};
  zet_core_callbacks_t epilogue_callbacks = {};
  prologue_callbacks.CommandList.pfnAppendLaunchKernelCb =
    OnEnterCommandListAppendLaunchKernel;
  epilogue_callbacks.CommandList.pfnAppendLaunchKernelCb =
    OnExitCommandListAppendLaunchKernel;

  status = zelTracerSetPrologues(tracer, &prologue_callbacks);
  assert(status == ZE_RESULT_SUCCESS);
  status = zelTracerSetEpilogues(tracer, &epilogue_callbacks);
  assert(status == ZE_RESULT_SUCCESS);
```
   `zet_core_callbacks_t` only covers the Level Zero 1.0 APIs. A newer API is registered with its own `zelTracer<Api>RegisterCallback` function from [zel_tracing_register_cb.h](https://github.com/oneapi-src/level-zero/blob/master/include/layers/zel_tracing_register_cb.h), after `zelTracerSetPrologues`/`zelTracerSetEpilogues` and before the tracer is enabled. For example, the SYCL runtime's Level Zero v2 adapter (the default on Intel(R) Arc(TM) B-Series GPUs) launches kernels with `zeCommandListAppendLaunchKernelWithArguments` (Level Zero 1.14):
```cpp
  status =
    zelTracerCommandListAppendLaunchKernelWithArgumentsRegisterCallback(
      tracer, ZEL_REGISTER_PROLOGUE,
      OnEnterCommandListAppendLaunchKernelWithArguments);
  assert(status == ZE_RESULT_SUCCESS);
  status =
    zelTracerCommandListAppendLaunchKernelWithArgumentsRegisterCallback(
      tracer, ZEL_REGISTER_EPILOGUE,
      OnExitCommandListAppendLaunchKernelWithArguments);
  assert(status == ZE_RESULT_SUCCESS);
```
   Limitations:
   - A loader only exports the registration functions of the APIs it knows (e.g. loader 1.24 has none for the 1.14 APIs). The PTI tools look them up at run time (`utils::ze::SetTracerCallbacks` in [ze_tracing_ext.h](../../utils/ze_tracing_ext.h)) and skip those that are missing, so they still load with older loaders.
   - Driver extension functions that an application gets with `zeDriverGetExtensionFunctionAddress` (e.g. `zexCounterBasedEventCreate2`) are called directly, bypassing the loader, and are not traced.

5. Enable tracing session. After this call you will be notified on every traced API call:
```cpp
  status = zelTracerSetEnabled(tracer, true);
  assert(status == ZE_RESULT_SUCCESS);
```
6. To stop tracing, disable it. This call is non-blocking so some callbacks may be executing even after this function successfully returned:
```cpp
  status = zelTracerSetEnabled(tracer, false);
  assert(status == ZE_RESULT_SUCCESS);
```
7. Destroy tracing handle to release all the resources and wait for all callbacks, which may be still in the process of execution:
```cpp
  status = zelTracerDestroy(tracer);
  assert(status == ZE_RESULT_SUCCESS);
```

## Build and Run
To make tracing work one need to link the application with Level Zero ICD library (e.g. `libze_loader.so`) and run it as following:
```
ZE_ENABLE_TRACING_LAYER=1 ./<application>
```

## Usage Details
- refer to oneAPI Level Zero API Tracing [documentation](https://oneapi-src.github.io/level-zero-spec/level-zero/latest/tools/PROG.html#api-tracing) to learn more

## Samples
- [Level Zero Hot Functions](../../samples/ze_hot_functions)
- [Level Zero Hot Kernels](../../samples/ze_hot_kernels)
- [Level Zero Debug Info](../../samples/ze_debug_info)
- [Level Zero Metric Query](../../samples/ze_metric_query)

## Tools
- [Level Zero Tracer](../../tools/ze_tracer)
- [Tracing and Profiling Tool for Data Parallel C++ (DPC++)](../../tools/onetrace)
- [GPU Metrics Collection Tool for Data Parallel C++ (DPC++)](../../tools/oneprof)