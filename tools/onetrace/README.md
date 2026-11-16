# Tracing and Profiling Tool for Data Parallel C++ (DPC++)
## Overview
This tool provides basic tracing and profiling capabilities for the compute applications based on Intel runtimes for OpenCL(TM) and Level Zero, like DPC++, Intel(R) Implicit SPMD Program Compiler (Intel(R) ISPC) and OpenMP* GPU offload programs.

The following capabilities are available:
```
Usage: ./onetrace[.exe] [options] <application> <args>
Options:
--call-logging [-c]            Trace host API calls
--host-timing  [-h]            Report host API execution time
--device-timing [-d]           Report kernels execution time
--kernel-submission [-s]       Report append (queued), submit and execute intervals for kernels
--device-timeline [-t]         Trace device activities
--chrome-call-logging          Dump host API calls to JSON file
--chrome-device-timeline       Dump device activities to JSON file per command queue
--chrome-kernel-timeline       Dump device activities to JSON file per kernel name
--chrome-device-stages         Dump device activities by stages to JSON file
--verbose [-v]                 Enable verbose mode to show more kernel information
--demangle                     Demangle DPC++ kernel names
--kernels-per-tile             Dump kernel information per tile
--tid                          Print thread ID into host API trace
--pid                          Print process ID into host API and device activity trace
--output [-o] <filename>       Print console logs into the file
--conditional-collection       Enable conditional collection mode
--version                      Print version
```

**Call Logging** mode allows to grab full host API trace, e.g.:
```
...
>>>> [2242678] zeDriverGet: pCount = 0x7ffeb39c4a7c (Count = 1) phDrivers = 0x1d949ce0 (hDrivers = 0)
<<<< [2244084] zeDriverGet [138 ns] Count = 1 hDrivers = 0x1d9290d8 -> ZE_RESULT_SUCCESS(0x0)
>>>> [2247744] zeDeviceGet: hDriver = 0x1d9290d8 pCount = 0x7ffeb39c4a5c (Count = 0) phDevices = 0
<<<< [2251286] zeDeviceGet [1496 ns] Count = 2 -> ZE_RESULT_SUCCESS(0x0)
>>>> [2253715] zeDeviceGet: hDriver = 0x1d9290d8 pCount = 0x7ffeb39c4a5c (Count = 2) phDevices = 0x1d949d00 (hDevices = 0)
<<<< [2255083] zeDeviceGet [150 ns] Count = 2 hDevices[0] = 0x1d92a0a8 hDevices[1] = 0x1d9323b8 -> ZE_RESULT_SUCCESS(0x0)
>>>> [2258890] zeDeviceGetProperties: hDevice = 0x1d92a0a8 pDeviceProperties = 0x7ffeb39c4a80
<<<< [2263642] zeDeviceGetProperties [3110 ns] -> ZE_RESULT_SUCCESS(0x0)
...
```
**Chrome Call Logging** mode dumps API calls to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool.

**Host Timing** mode collects duration for each API call and provides the summary for the whole application:
```
=== API Timing Results: ===

             Total Execution Time (ns):             49041636
    Total API Time for L0 backend (ns):             29108531
Total API Time for CL CPU backend (ns):                24029
Total API Time for CL GPU backend (ns):                10761

== L0 Backend: ==

                              Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
          zeCommandListHostSynchronize,           8,            13452068,     46.21,             1681508,                 520,             3375809
         zeCommandListAppendMemoryCopy,          16,             8208144,     28.20,              513009,              160425,              658785
                        zeModuleCreate,           1,             3073348,     10.56,             3073348,             3073348,             3073348
          zeCommandListCreateImmediate,           3,             1389025,      4.77,              463008,               98587,             1026128
                zeEventHostSynchronize,           8,             1093813,      3.76,              136726,                 762,              318932
           zeContextMakeMemoryResident,          14,              809873,      2.78,               57848,                7600,              348491
       zeCommandListAppendLaunchKernel,           4,              281267,      0.97,               70316,               15012,              209762
                      zeMemAllocShared,           8,              214196,      0.74,               26774,               21233,               49435
                             zeMemFree,          14,              177960,      0.61,               12711,                6165,               25066
                      zeMemAllocDevice,           7,              124695,      0.43,               17813,                6461,               29097
                        zeMemAllocHost,           2,               90850,      0.31,               45425,               20612,               70238
                         zeEventCreate,          20,               46249,      0.16,                2312,                 667,               13107
           zeDeviceGetGlobalTimestamps,           4,               36910,      0.13,                9227,                7911,               11243
           zeEventQueryKernelTimestamp,           8,               16930,      0.06,                2116,                 862,                4466
...
```
**Device Timing** mode collects duration for each kernel on the device and provides the summary for the whole application:

Memory transfers for Level Zero are supplemented by transfer direction:
- "M" - system memory allocated with malloc or new;
- "H" - USM host memory allocated with `zeMemAllocHost`;
- "D" - USM device memory allocated with `zeMemAllocDevice`;
- "S" - USM shared memory allocated with `zeMemAllocShared`;
```
=== Device Timing Results: ===

                Total Execution Time (ns):             56702065
    Total Device Time for L0 backend (ns):             21734720

== L0 Backend: ==

                                                                                              Kernel,       Calls,           Time (ns),    Time (%),        Average (ns),            Min (ns),            Max (ns)
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM,           4,            13393920,       61.62,             3348480,             3336000,             3363520
                                                                  zeCommandListAppendMemoryCopy(M2D),          12,             6937120,       31.92,              578093,              542800,              635600
                                                                  zeCommandListAppendMemoryCopy(D2M),           4,             1403680,        6.46,              350920,              336880,              392080
```
**Kernel Submission** mode collects append (queued for OpenCL(TM)), submit and execute intervals for kernels and memory transfers:
```
=== Kernel Submission Results: ===

                Total Execution Time (ns):             57173585
    Total Device Time for L0 backend (ns):             21824080

== L0 Backend: ==

                                                                                              Kernel,       Calls,         Append (ns),  Append (%),         Submit (ns),  Submit (%),        Execute (ns), Execute (%),
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM,           4,                   0,        -nan,              358560,       28.16,            13352320,       61.18,
                                                                  zeCommandListAppendMemoryCopy(M2D),          12,                   0,        -nan,              185280,       14.55,             7060400,       32.35,
                                                                  zeCommandListAppendMemoryCopy(D2M),           4,                   0,        -nan,              729600,       57.29,             1411360,        6.47,
```
**Verbose** mode provides additional information per kernel (SIMD width, group count and group size for oneAPI Level Zero (Level Zero) and SIMD width, global and local size for OpenCL(TM)) and per transfer (bytes transferred). This option should be used in addition to others, e.g. for **Device Timing** mode one can get:
```
=== Device Timing Results: ===

                Total Execution Time (ns):             49191900
    Total Device Time for L0 backend (ns):             21602800

== L0 Backend: ==

                                                                                                                                Kernel,       Calls,           Time (ns),    Time (%),        Average (ns),            Min (ns),            Max (ns)
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],           4,            13354560,       61.82,             3338640,             3335360,             3341440
                                                                                     zeCommandListAppendMemoryCopy(M2D)[4194304 bytes],          12,             6837920,       31.65,              569826,              541840,              610400
                                                                                     zeCommandListAppendMemoryCopy(D2M)[4194304 bytes],           4,             1410320,        6.53,              352580,              335920,              396960
```

**Device Timeline** mode dumps four timestamps for each device activity - *queued* to the host command queue for OpenCL(TM) or "append" to the command list for Level Zero, *submit* to device queue, *start* and *end* on the device (all the timestamps are in CPU nanoseconds):
```
Device Timeline: start time (CLOCK_MONOTONIC_RAW) [ns] = 63808884777851
Device Timeline: start time (CLOCK_MONOTONIC) [ns] = 63816389603957
Device Timeline: start time (CLOCK_REALTIME) [ns] = 1794831335073727339
Device Timeline (queue: 0x106c8898): zeCommandListAppendMemoryCopy(M2D)<1.1> [ns] = 18225355 (append) 18225355 (submit) 18275675 (start) 18879515 (end)
Device Timeline (queue: 0x106c8898): zeCommandListAppendMemoryCopy(M2D)<2.1> [ns] = 18986310 (append) 18986310 (submit) 19013190 (start) 19739030 (end)
Device Timeline (queue: 0x106c8898): zeCommandListAppendMemoryCopy(M2D)<3.1> [ns] = 19860289 (append) 19860289 (submit) 19885569 (start) 20504369 (end)
Device Timeline (queue: 0x106c8898): zeCommandListAppendMemoryCopy(D2M)<5.1> [ns] = 27970973 (append) 27970973 (submit) 28359213 (start) 28771933 (end)
Device Timeline (queue: 0x106ef448): _ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM<4.1> [ns] = 24281690 (append) 24281690 (submit) 24562730 (start) 27902090 (end)
Device Timeline (queue: 0x106c8898): zeCommandListAppendMemoryCopy(M2D)<6.1> [ns] = 29442271 (append) 29442271 (submit) 29452191 (start) 30035391 (end)
...
```
**Chrome Device Timeline** mode dumps timestamps for device activities per command queue to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool. Can't be used with **Chrome Kernel Timeline** and **Chrome Device Stages**.

**Chrome Kernel Timeline** mode dumps timestamps for device activities per kernel name to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool. Can't be used with **Chrome Device Timeline**.

**Chrome Device Stages** mode provides alternative view for device queue where each kernel invocation is divided into stages: "queued" or "appended", "sumbitted" and "execution". Can't be used with **Chrome Device Timeline**.

**Conditional Collection** mode allows one to enable data collection for any target interval (by default collection will be disabled) using environment variable `PTI_ENABLE_COLLECTION`, e.g.:
```cpp
// Collection disabled
setenv("PTI_ENABLE_COLLECTION", "1", 1);
// Collection enabled
unsetenv("PTI_ENABLE_COLLECTION");
// Collection disabled
```
All the API calls and kernels, which submission happens while collection disabled interval, will be omitted from final results.

**Traced Level Zero APIs:** Level Zero call logging and host timing (`-c`, `-h`, `--chrome-call-logging`) cover every API of the Level Zero headers that the tool is built with: the callbacks are generated from them at build time (`../ze_tracer/gen_tracing_callbacks.py`). The 1.0 APIs are registered with `zelTracerSetPrologues/Epilogues`. Newer ones, e.g. `zeCommandListAppendLaunchKernelWithArguments`, which the SYCL Level Zero v2 adapter uses for kernel launches, are registered with the per-API `zelTracer*RegisterCallback` functions of the loader. These are looked up at run time, so APIs that the installed loader doesn't know are skipped. Driver extension functions that an application gets with `zeDriverGetExtensionFunctionAddress` (e.g. `zexCounterBasedEventCreate2`) are called directly, bypassing the loader, and are not traced.

## Supported OS
- Linux
- Windows (*under development*)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver) and Intel(R) Arc(TM) B570 Graphics (xe kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [OpenCL(TM) ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime) to run on GPU
- [Intel(R) Xeon(R) Processor / Intel(R) Core(TM) Processor (CPU) Runtimes](https://www.intel.com/content/www/us/en/developer/articles/tool/opencl-drivers.html#cpu-section) to run on CPU

**OpenCL tracing extension:** OpenCL tracing (OpenCL API calls and kernels) uses the Intel tracing extension (`clCreateTracingHandleINTEL` and related functions), which the tool looks up with `clGetExtensionFunctionAddressForPlatform`. It isn't listed in the device extension strings. The Intel OpenCL runtimes for GPU (compute-runtime, checked with 25.18 and 26.05) and for CPU provide it. With a runtime that doesn't, the tool prints `[WARNING] The OpenCL runtime does not provide the Intel tracing extension (clCreateTracingHandleINTEL)` and collects no OpenCL data; Level Zero tracing is not affected.

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/tools/onetrace
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./onetrace [options] <target_application>
```
One may use e.g. [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
./onetrace -c -h ../../../samples/dpc_gemm/build/dpc_gemm cpu
./onetrace -c -h ../../../samples/dpc_gemm/build/dpc_gemm gpu
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\tools\onetrace
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
onetrace.exe [options] <target_application>
```
One may use e.g. [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
onetrace.exe -c -h ..\..\..\samples\dpc_gemm\build\dpc_gemm.exe cpu
onetrace.exe -c -h ..\..\..\samples\dpc_gemm\build\dpc_gemm.exe gpu
```
