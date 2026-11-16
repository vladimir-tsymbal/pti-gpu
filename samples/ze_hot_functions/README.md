# Level Zero Hot Functions
## Overview
This sample is a simple LD_PRELOAD based tool that allows to collect all called Level Zero API functions within an application along with their total execution time and call count.

As a result, table like the following will be printed.
```
=== API Timing Results: ===

Total Execution Time (ns): 32438915
Total API Time (ns): 19105516

                         Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
        zeCommandQueueSynchronize,           4,            16221967,     84.91,             4055491,             3972716,             4278531
    zeCommandListAppendMemoryCopy,          12,             1625044,      8.51,              135420,               73895,              532550
             zeCommandQueueCreate,           4,              260556,      1.36,               65139,                1690,              254868
                   zeModuleCreate,           1,              228178,      1.19,              228178,              228178,              228178
zeCommandQueueExecuteCommandLists,           4,              221465,      1.16,               55366,               24979,              132868
                 zeMemAllocDevice,          12,               99055,      0.52,                8254,                3739,               19250
             zeCommandListDestroy,           4,               98112,      0.51,               24528,               21303,               30764
                        zeMemFree,          12,               75062,      0.39,                6255,                5018,               11562
                zeEventPoolCreate,           4,               69158,      0.36,               17289,               14545,               21327
               zeEventPoolDestroy,           4,               53158,      0.28,               13289,               12176,               15530
              zeCommandListCreate,           4,               44797,      0.23,               11199,                3544,               33138
                    zeEventCreate,           4,               26347,      0.14,                6586,                5785,                7363
      zeEventQueryKernelTimestamp,           4,               15724,      0.08,                3931,                3250,                4992
  zeCommandListAppendLaunchKernel,           4,               11022,      0.06,                2755,                1932,                4628
         zeKernelSetArgumentValue,          16,                8600,      0.05,                 537,                 166,                2706
            zeDeviceGetProperties,           9,                7380,      0.04,                 820,                 216,                2213
                    zeInitDrivers,           1,                6746,      0.04,                6746,                6746,                6746
                   zeKernelCreate,           1,                6108,      0.03,                6108,                6108,                6108
               zeCommandListClose,           4,                4818,      0.03,                1204,                 938,                1884
         zeKernelSuggestGroupSize,           4,                4733,      0.02,                1183,                 636,                2181
                  zeModuleDestroy,           1,                3940,      0.02,                3940,                3940,                3940
       zeCommandListAppendBarrier,           8,                2795,      0.01,                 349,                 129,                1127
                  zeContextCreate,           1,                2269,      0.01,                2269,                2269,                2269
            zeCommandQueueDestroy,           4,                1975,      0.01,                 493,                 359,                 712
             zeKernelSetGroupSize,           4,                1584,      0.01,                 396,                 206,                 600
                   zeEventDestroy,           4,                1211,      0.01,                 302,                 221,                 488
                  zeKernelDestroy,           1,                1138,      0.01,                1138,                1138,                1138
                      zeDeviceGet,           4,                 891,      0.00,                 222,                  55,                 642
                 zeContextDestroy,           1,                 838,      0.00,                 838,                 838,                 838
                      zeDriverGet,           4,                 783,      0.00,                 195,                  37,                 508
            zeDeviceGetSubDevices,           1,                  62,      0.00,                  62,                  62,                  62
```
**Traced Level Zero APIs:** The timing results cover every API of the Level Zero headers that the tool is built with: the callbacks are generated from them at build time (`tools/ze_tracer/gen_tracing_callbacks.py --hot-functions`). The 1.0 APIs are registered with `zelTracerSetPrologues/Epilogues`. Newer ones, e.g. `zeCommandListAppendLaunchKernelWithArguments`, which the SYCL Level Zero v2 adapter uses for kernel launches, are registered with the per-API `zelTracer*RegisterCallback` functions of the loader. These are looked up at run time, so APIs that the installed loader doesn't know are skipped. Driver extension functions that an application gets with `zeDriverGetExtensionFunctionAddress` (e.g. `zexCounterBasedEventCreate2`) are called directly, bypassing the loader, and are not traced.

## Supported OS
- Linux
- Windows (*under development*)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/ze_hot_functions
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./ze_hot_functions <target_application>
```
One may use [ze_gemm](../ze_gemm) or [dpc_gemm](../dpc_gemm) as target application:
```sh
./ze_hot_functions ../../ze_gemm/build/ze_gemm
./ze_hot_functions ../../dpc_gemm/build/dpc_gemm
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\ze_hot_functions
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the tool:
```sh
ze_hot_functions.exe <target_application>
```
One may use [ze_gemm](../ze_gemm) or [dpc_gemm](../dpc_gemm) as target application:
```sh
ze_hot_functions.exe ..\..\ze_gemm\build\ze_gemm.exe
ze_hot_functions.exe ..\..\dpc_gemm\build\dpc_gemm.exe
```