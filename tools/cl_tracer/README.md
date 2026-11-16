# OpenCL(TM) Tracer
## Overview
This tool is an analogue of [Intercept Layer for OpenCL(TM) Applications](https://github.com/intel/opencl-intercept-layer) designed based on internal [tracing mechanism](../../chapters/runtime_api_tracing/OpenCL.md) implemented in Intel runtimes for OpenCL(TM).

The following capabilities are available currently:
```
Usage: ./cl_tracer[.exe] [options] <application> <args>
Options:
--call-logging [-c]            Trace host API calls
--host-timing  [-h]            Report host API execution time
--device-timing [-d]           Report kernels execution time
--kernel-submission [-s]       Report queued, submit and execute intervals for kernels
--device-timeline [-t]         Trace device activities
--chrome-call-logging          Dump host API calls to JSON file
--chrome-device-timeline       Dump device activities to JSON file per command queue
--chrome-kernel-timeline       Dump device activities to JSON file per kernel name
--chrome-device-stages         Dump device activities by stages to JSON file
--verbose [-v]                 Enable verbose mode to show more kernel information
--demangle                     Demangle DPC++ kernel names
--tid                          Print thread ID into host API trace
--pid                          Print process ID into host API and device activity trace
--output [-o] <filename>       Print console logs into the file
--conditional-collection       Enable conditional collection mode
--version                      Print version
```

**Call Logging** mode allows to grab full host API trace, e.g.:
```
...
>>>> [107857] clGetDeviceInfo: device = 0x5681c9daa0d0 paramName = 4140 paramValueSize = 1024 paramValue = 0x7ffc55b23080 paramValueSizeRet = 0
<<<< [109253] clGetDeviceInfo [70 ns] -> CL_SUCCESS (0)
<<<< [121667] clGetDeviceInfo [129 ns] -> CL_SUCCESS (0)
>>>> [6356473] clCreateContext: properties = 0 numDevices = 1 devices = 0x7ffc55b23628 funcNotify = 0 userData = 0 errcodeRet = 0x7ffc55b2361c
<<<< [6383429] clCreateContext [14291 ns] result = 0x5681c88f0b70 -> CL_SUCCESS (0)
>>>> [6388494] clCreateCommandQueueWithProperties: context = 0x5681c88f0b70 device = 0x5681c9daa0d0 properties = 0x7ffc55b236d0 errcodeRet = 0x7ffc55b2361c
<<<< [6398261] clCreateCommandQueueWithProperties [7353 ns] result = 0x5681c88f1330 -> CL_SUCCESS (0)
>>>> [6408291] clCreateProgramWithSource: context = 0x5681c88f0b70 count = 1 strings = 0x5681ae6d4010 lengths = 0 errcodeRet = 0x7ffc55b2361c
...
```
**Chrome Call Logging** mode dumps API calls to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool.

**Host Timing** mode collects duration for each API call and provides the summary for the whole application:
```
=== API Timing Results: ===

          Total Execution Time (ns):             32775017
Total API Time for CPU backend (ns):                  358
Total API Time for GPU backend (ns):             19591631

== CPU Backend: ==

      Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
clGetDeviceIDs,           1,                 358,    100.00,                 358,                 358,                 358

== GPU Backend: ==

                          Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
                          clFinish,           4,            14814345,     75.62,             3703586,             3488856,             3833858
              clEnqueueWriteBuffer,           8,             2165171,     11.05,              270646,               83618,             1177368
               clEnqueueReadBuffer,           4,             1616550,      8.25,              404137,              303211,              542093
            clEnqueueNDRangeKernel,           4,              424069,      2.16,              106017,               21834,              337816
                    clBuildProgram,           1,              239573,      1.22,              239573,              239573,              239573
                clReleaseMemObject,          12,              124560,      0.64,               10380,                5409,               27848
                    clCreateBuffer,          12,              117285,      0.60,                9773,                3705,               29603
           clGetEventProfilingInfo,           8,               19752,      0.10,                2469,                 124,                6618
                  clReleaseProgram,           1,               14679,      0.07,               14679,               14679,               14679
                   clCreateContext,           1,               14134,      0.07,               14134,               14134,               14134
...
```
**Device Timing** mode collects duration for each kernel on the device and provides the summary for the whole application:
```
=== Device Timing Results: ===

             Total Execution Time (ns):             31991623
Total Device Time for CPU backend (ns):                    0
Total Device Time for GPU backend (ns):             16291520

== GPU Backend: ==

              Kernel,       Calls,           Time (ns),    Time (%),        Average (ns),            Min (ns),            Max (ns)
                GEMM,           4,            13355520,       81.98,             3338880,             3336960,             3341600
clEnqueueWriteBuffer,           8,             1985680,       12.19,              248210,              195280,              305040
 clEnqueueReadBuffer,           4,              950320,        5.83,              237580,              204160,              337760
```
**Kernel Submission** mode collects queued, submit and execute intervals for kernels and memory transfers:
```
=== Kernel Submission Results: ===

             Total Execution Time (ns):             33519364
Total Device Time for CPU backend (ns):                    0
Total Device Time for GPU backend (ns):             17123360

== GPU Backend: ==

              Kernel,       Calls,         Queued (ns),  Queued (%),         Submit (ns),  Submit (%),        Execute (ns), Execute (%),
                GEMM,           4,              245414,       53.90,             1562800,       60.35,            13352000,       77.98,
clEnqueueWriteBuffer,           8,              199453,       43.80,              991840,       38.30,             2412880,       14.09,
 clEnqueueReadBuffer,           4,               10458,        2.30,               35120,        1.36,             1358480,        7.93,
```
**Verbose** mode provides additional information per kernel (SIMD width, global and local sizes) and per transfer (bytes transferred). This option should be used in addition to others, e.g. for **Device Timing** mode one can get:
```
=== Device Timing Results: ===

             Total Execution Time (ns):             33413781
Total Device Time for CPU backend (ns):                    0
Total Device Time for GPU backend (ns):             17132560

== GPU Backend: ==

                                                 Kernel,       Calls,           Time (ns),    Time (%),        Average (ns),            Min (ns),            Max (ns)
                 GEMM[SIMD32 {1024; 1024; 1} {0; 0; 0}],           4,            13356000,       77.96,             3339000,             3336960,             3340480
clEnqueueWriteBufferclEnqueueWriteBuffer[4194304 bytes],           8,             2417440,       14.11,              302180,              299520,              304880
  clEnqueueReadBufferclEnqueueReadBuffer[4194304 bytes],           4,             1359120,        7.93,              339780,              338960,              340880
```

**Device Timeline** mode dumps four timestamps for each device activity - *queued* to the host command queue, *submit* to device queue, *start* and *end* on the device (all the timestamps are in CPU nanoseconds):
```
Device Timeline: start time (CLOCK_MONOTONIC_RAW) [ns] = 63844238092827
Device Timeline: start time (CLOCK_MONOTONIC) [ns] = 63851746238618
Device Timeline: start time (CLOCK_REALTIME) [ns] = 1794831370430361998
Device Timeline (queue: 0x5b54c947b270): clEnqueueWriteBuffer<1> [ns] = 7534241 (queued) 7712661 (submit) 7926421 (start) 8225461 (end)
Device Timeline (queue: 0x5b54c947b270): clEnqueueWriteBuffer<2> [ns] = 8077003 (queued) 8081018 (submit) 8230698 (start) 8534058 (end)
Device Timeline (queue: 0x5b54c947b270): GEMM<3> [ns] = 8125706 (queued) 8350948 (submit) 8535268 (start) 11879268 (end)
Device Timeline (queue: 0x5b54c947b270): clEnqueueReadBuffer<4> [ns] = 12157851 (queued) 12162176 (submit) 12173456 (start) 12511616 (end)
Device Timeline (queue: 0x5b54c947b270): clEnqueueWriteBuffer<5> [ns] = 14352049 (queued) 14357666 (submit) 14385106 (start) 14690146 (end)
Device Timeline (queue: 0x5b54c947b270): clEnqueueWriteBuffer<6> [ns] = 14493360 (queued) 14494773 (submit) 14692693 (start) 14994693 (end)
...
```
**Chrome Device Timeline** mode dumps timestamps for device activities per command queue to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool. Can't be used with **Chrome Kernel Timeline** and **Chrome Device Stages**.

**Chrome Kernel Timeline** mode dumps timestamps for device activities per kernel name to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool. Can't be used with **Chrome Device Timeline**.

**Chrome Device Stages** mode provides alternative view for device queue where each kernel invocation is divided into stages: "queued", "sumbitted" and "execution". Can't be used with **Chrome Device Timeline**.

**Conditional Collection** mode allows one to enable data collection for any target interval (by default collection will be disabled) using environment variable `PTI_ENABLE_COLLECTION`, e.g.:
```cpp
// Collection disabled
setenv("PTI_ENABLE_COLLECTION", "1", 1);
// Collection enabled
unsetenv("PTI_ENABLE_COLLECTION");
// Collection disabled
```
All the API calls and kernels, which submission happens while collection disabled interval, will be omitted from final results.

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

**OpenCL tracing extension:** OpenCL tracing uses the Intel tracing extension (`clCreateTracingHandleINTEL` and related functions), which the tool looks up with `clGetExtensionFunctionAddressForPlatform`. It isn't listed in the device extension strings. The Intel OpenCL runtimes for GPU (compute-runtime, checked with 25.18 and 26.05) and for CPU provide it. With a runtime that doesn't, the tool prints `[WARNING] The OpenCL runtime does not provide the Intel tracing extension (clCreateTracingHandleINTEL)` and collects no OpenCL data.

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/tools/cl_tracer
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./cl_tracer [options] <target_application>
```
One may use [cl_gemm](../../samples/cl_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
./cl_tracer -c -h ../../../samples/cl_gemm/build/cl_gemm
./cl_tracer -c -h ../../../samples/dpc_gemm/build/dpc_gemm cpu
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\tools\cl_tracer
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
cl_tracer.exe [options] <target_application>
```
One may use [cl_gemm](../../samples/cl_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
cl_tracer.exe -c -h ..\..\..\samples\cl_gemm\build\cl_gemm.exe
cl_tracer.exe -c -h ..\..\..\samples\dpc_gemm\build\Release\dpc_gemm.exe cpu
```
