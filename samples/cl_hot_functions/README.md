# OpenCL(TM) Hot Functions
## Overview
This is a simple LD_PRELOAD based tool that allows to collect all called OpenCL(TM) API functions within an application along with their total execution time and call count.

As a result, table like the following will be printed.
```
=== API Timing Results: ===

Total Execution Time (ns): 32635143
Total API Time for CPU backend (ns): 580
Total API Time for GPU backend (ns): 19302301

== CPU Backend: ==

      Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
clGetDeviceIDs,           1,                 580,    100.00,                 580,                 580,                 580

== GPU Backend: ==

                          Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
                          clFinish,           4,            14458751,     74.91,             3614687,             3419046,             3785200
              clEnqueueWriteBuffer,           8,             2224270,     11.52,              278033,              116577,             1060662
               clEnqueueReadBuffer,           4,             1640472,      8.50,              410118,              334629,              626704
            clEnqueueNDRangeKernel,           4,              432830,      2.24,              108207,               20704,              356843
                    clBuildProgram,           1,              240834,      1.25,              240834,              240834,              240834
                clReleaseMemObject,          12,              114969,      0.60,                9580,                5045,               26010
                    clCreateBuffer,          12,              102672,      0.53,                8556,                3463,               27595
           clGetEventProfilingInfo,           8,               19095,      0.10,                2386,                  99,                6581
                  clReleaseProgram,           1,               15021,      0.08,               15021,               15021,               15021
clCreateCommandQueueWithProperties,           1,               10730,      0.06,               10730,               10730,               10730
                   clCreateContext,           1,               10639,      0.06,               10639,               10639,               10639
                    clCreateKernel,           1,                6607,      0.03,                6607,                6607,                6607
                    clSetKernelArg,          16,                6579,      0.03,                 411,                  94,                2310
         clCreateProgramWithSource,           1,                4874,      0.03,                4874,                4874,                4874
                    clReleaseEvent,           4,                4329,      0.02,                1082,                 728,                1760
                  clReleaseContext,           1,                2891,      0.01,                2891,                2891,                2891
             clReleaseCommandQueue,           1,                2637,      0.01,                2637,                2637,                2637
                   clReleaseKernel,           1,                2032,      0.01,                2032,                2032,                2032
                   clGetKernelInfo,           4,                1348,      0.01,                 337,                 207,                 540
                   clGetDeviceInfo,           4,                 394,      0.00,                  98,                  54,                 164
                    clGetDeviceIDs,           2,                 327,      0.00,                 163,                 159,                 168
```
## Supported OS
- Linux
- Windows (not validated)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver) and Intel(R) Arc(TM) B570 Graphics (xe kernel driver); on CPU, with the Intel(R) OpenCL(TM) CPU runtime of oneAPI 2026.2; other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [OpenCL(TM) ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime) to run on GPU
- [Intel(R) Xeon(R) Processor / Intel(R) Core(TM) Processor (CPU) Runtimes](https://www.intel.com/content/www/us/en/developer/articles/tool/opencl-drivers.html#cpu-section) to run on CPU

**OpenCL tracing extension:** OpenCL tracing uses the Intel tracing extension (`clCreateTracingHandleINTEL` and related functions), which the sample looks up with `clGetExtensionFunctionAddressForPlatform`. It isn't listed in the device extension strings. The Intel OpenCL runtimes for GPU (compute-runtime, checked with 25.18 and 26.05) and for CPU provide it. With a runtime that doesn't, the sample prints `[WARNING] The OpenCL runtime does not provide the Intel tracing extension (clCreateTracingHandleINTEL)` and collects no OpenCL data.

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/cl_hot_functions
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./cl_hot_functions <target_application>
```
One may use [cl_gemm](../cl_gemm) or [dpc_gemm](../dpc_gemm) as target application:
```sh
./cl_hot_functions ../../cl_gemm/build/cl_gemm
./cl_hot_functions ../../dpc_gemm/build/dpc_gemm cpu
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_hot_functions
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
cl_hot_functions.exe <target_application>
```
One may use [cl_gemm](../cl_gemm) or [dpc_gemm](../dpc_gemm) as target application:
```sh
cl_hot_functions.exe ..\..\cl_gemm\build\cl_gemm.exe
cl_hot_functions.exe ..\..\dpc_gemm\build\Release\dpc_gemm.exe cpu
```