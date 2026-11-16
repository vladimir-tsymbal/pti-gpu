# OpenCL(TM) Hot Functions
## Overview
This is a simple LD_PRELOAD based tool that allows to collect OpenCL(TM) kernels within an application along with their total execution time and call count.

As a result, table like the following will be printed.
```
=== Device Timing Results: ===

Total Execution Time (ns): 34067725
Total Device Time for CPU (ns): 0
Total Device Time for GPU (ns): 13356960

== GPU Backend: ==

    Kernel,       Calls, SIMD,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
      GEMM,           4,   32,            13356960,    100.00,             3339240,             3336960,             3340800
```
## Supported OS
- Linux
- Windows (not validated)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); on CPU, with the Intel(R) OpenCL(TM) CPU runtime of oneAPI 2026.2; other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [OpenCL(TM) ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime) to run on GPU
- [Intel(R) Xeon(R) Processor / Intel(R) Core(TM) Processor (CPU) Runtimes](https://www.intel.com/content/www/us/en/developer/articles/tool/opencl-drivers.html#cpu-section) to run on CPU

**OpenCL tracing extension:** OpenCL tracing uses the Intel tracing extension (`clCreateTracingHandleINTEL` and related functions), which the sample looks up with `clGetExtensionFunctionAddressForPlatform`. It isn't listed in the device extension strings. The Intel OpenCL runtimes for GPU (compute-runtime, checked with 25.18, 26.05 and 26.40) and for CPU provide it. With a runtime that doesn't, the sample prints `[WARNING] The OpenCL runtime does not provide the Intel tracing extension (clCreateTracingHandleINTEL)` and collects no OpenCL data.

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/cl_hot_kernels
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./cl_hot_kernels <target_application>
```
One may use [cl_gemm](../cl_gemm) or [dpc_gemm](../dpc_gemm) as target application:
```sh
./cl_hot_kernels ../../cl_gemm/build/cl_gemm
./cl_hot_kernels ../../dpc_gemm/build/dpc_gemm cpu
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_hot_kernels
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
cl_hot_kernels.exe <target_application>
```
One may use [cl_gemm](../cl_gemm) or [dpc_gemm](../dpc_gemm) as target application:
```sh
cl_hot_kernels.exe ..\..\cl_gemm\build\cl_gemm.exe
cl_hot_kernels.exe ..\..\dpc_gemm\build\Release\dpc_gemm.exe cpu
```