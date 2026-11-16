# Level Zero Tracer
## Overview
This tool is an analogue of [Intercept Layer for OpenCL(TM) Applications](https://github.com/intel/opencl-intercept-layer) designed to support Level Zero.

The following capabilities are available currently:
```
Usage: ./ze_tracer[.exe] [options] <application> <args>
Options:
--call-logging [-c]            Trace host API calls
--host-timing  [-h]            Report host API execution time
--device-timing [-d]           Report kernels execution time
--kernel-submission [-s]       Report append, submit and execute intervals for kernels
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
>>>> [223698] zeDeviceGetProperties: hDevice = 0x5f4429b17b18 pDeviceProperties = 0x7ffc5ced81f0
<<<< [228341] zeDeviceGetProperties [2519 ns] -> ZE_RESULT_SUCCESS(0x0)
>>>> [241841] zeDeviceGetProperties: hDevice = 0x5f4429b1fe98 pDeviceProperties = 0x7ffc5ced81f0
<<<< [243373] zeDeviceGetProperties [595 ns] -> ZE_RESULT_SUCCESS(0x0)
>>>> [249798] zeDeviceGetSubDevices: hDevice = 0x5f4429b17b18 pCount = 0x7ffc5ced8170 (Count = 0) phSubdevices = 0
<<<< [251385] zeDeviceGetSubDevices [118 ns] Count = 0 -> ZE_RESULT_SUCCESS(0x0)
>>>> [254355] zeDriverGet: pCount = 0x7ffc5ced81b0 (Count = 0) phDrivers = 0
<<<< [255422] zeDriverGet [77 ns] Count = 1 -> ZE_RESULT_SUCCESS(0x0)
...
```
**Chrome Call Logging** mode dumps API calls to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool.

**Host Timing** mode collects duration for each API call and provides the summary for the whole application:
```
=== API Timing Results: ===

Total Execution Time (ns):             33473481
      Total API Time (ns):             20286046

                         Function,       Calls,           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
        zeCommandQueueSynchronize,           4,            17017085,     83.89,             4254271,             3984793,             4434833
    zeCommandListAppendMemoryCopy,          12,             1735239,      8.55,              144603,               73218,              513841
zeCommandQueueExecuteCommandLists,           4,              396728,      1.96,               99182,               24373,              204972
             zeCommandQueueCreate,           4,              246881,      1.22,               61720,                1619,              239243
                   zeModuleCreate,           1,              202525,      1.00,              202525,              202525,              202525
                 zeMemAllocDevice,          12,              177774,      0.88,               14814,                3672,               65577
             zeCommandListDestroy,           4,              113722,      0.56,               28430,               20947,               39396
                        zeMemFree,          12,               82011,      0.40,                6834,                4909,               14333
                zeEventPoolCreate,           4,               78271,      0.39,               19567,               14165,               23162
               zeEventPoolDestroy,           4,               59064,      0.29,               14766,               11934,               18064
              zeCommandListCreate,           4,               50622,      0.25,               12655,                3146,               35981
                    zeEventCreate,           4,               30152,      0.15,                7538,                6145,                8842
      zeEventQueryKernelTimestamp,           4,               18146,      0.09,                4536,                3314,                6623
  zeCommandListAppendLaunchKernel,           4,               10803,      0.05,                2700,                1744,                4254
                  zeContextCreate,           1,               10785,      0.05,               10785,               10785,               10785
         zeKernelSetArgumentValue,          16,                8831,      0.04,                 551,                 142,                2576
                    zeInitDrivers,           1,                6789,      0.03,                6789,                6789,                6789
            zeDeviceGetProperties,           9,                6600,      0.03,                 733,                 151,                1647
...
```
**Device Timing** mode collects duration for each kernel on the device and provides the summary for the whole application.

Memory transfers are supplemented by transfer direction:
- "M" - system memory allocated with malloc or new;
- "H" - USM host memory allocated with `zeMemAllocHost`;
- "D" - USM device memory allocated with `zeMemAllocDevice`;
- "S" - USM shared memory allocated with `zeMemAllocShared`;
```
=== Device Timing Results: ===

Total Execution Time (ns):             34283511
   Total Device Time (ns):             34357280

                            Kernel,       Calls,           Time (ns),    Time (%),        Average (ns),            Min (ns),            Max (ns)
        zeCommandListAppendBarrier,           8,            15593920,       45.39,             1949240,              418240,             3337280
                              GEMM,           4,            13338720,       38.82,             3334680,             3333760,             3335680
zeCommandListAppendMemoryCopy(M2D),           8,             4477600,       13.03,              559700,              415360,              712480
zeCommandListAppendMemoryCopy(D2M),           4,              947040,        2.76,              236760,              203680,              331200
```
**Kernel Submission** mode collects append, submit and execute intervals for kernels and memory transfers:
```
=== Kernel Submission Results: ===

Total Execution Time (ns):             34754016
   Total Device Time (ns):             35434720

                            Kernel,       Calls,         Append (ns),  Append (%),         Submit (ns),  Submit (%),        Execute (ns), Execute (%),
        zeCommandListAppendBarrier,           8,             1760281,       22.30,             3182880,       13.91,            15912160,       44.91,
                              GEMM,           4,              806259,       10.21,             2873120,       12.55,            13340320,       37.65,
zeCommandListAppendMemoryCopy(M2D),           8,             4556910,       57.72,              608800,        2.66,             5110080,       14.42,
zeCommandListAppendMemoryCopy(D2M),           4,              771164,        9.77,            16223200,       70.88,             1072160,        3.03,
```
**Verbose** mode provides additional information per kernel (SIMD width, group count and group size) and per transfer (bytes transferred). This option should be used in addition to others, e.g. for **Device Timing** mode one can get:
```
=== Device Timing Results: ===

Total Execution Time (ns):             34272257
   Total Device Time (ns):             34995040

                                           Kernel,       Calls,           Time (ns),    Time (%),        Average (ns),            Min (ns),            Max (ns)
                       zeCommandListAppendBarrier,           8,            15803200,       45.16,             1975400,              524800,             3338400
           GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],           4,            13342080,       38.13,             3335520,             3334560,             3336800
zeCommandListAppendMemoryCopy(M2D)[4194304 bytes],           8,             4887520,       13.97,              610940,              521760,              871680
zeCommandListAppendMemoryCopy(D2M)[4194304 bytes],           4,              962240,        2.75,              240560,              232640,              248800
```

**Device Timeline** mode (***Linux kernel 5.0+ is required for accurate measurements***) dumps four timestamps for each device activity - *append* to the command list, *submit* to device queue, *start* and *end* on the device (all the timestamps are in CPU nanoseconds):
```
Device Timeline: start time (CLOCK_MONOTONIC_RAW) [ns] = 63826416939706
Device Timeline: start time (CLOCK_MONOTONIC) [ns] = 63833923415213
Device Timeline: start time (CLOCK_REALTIME) [ns] = 1794831352607538592
Device Timeline (queue: 0x5d8082b7bad8): zeCommandListAppendMemoryCopy(M2D)<1.1> [ns] = 6573671 (append) 8957076 (submit) 9165396 (start) 10048596 (end)
Device Timeline (queue: 0x5d8082b7bad8): zeCommandListAppendMemoryCopy(M2D)<2.1> [ns] = 8257188 (append) 8957076 (submit) 9166036 (start) 10048916 (end)
Device Timeline (queue: 0x5d8082b7bad8): zeCommandListAppendBarrier<3.1> [ns] = 8448815 (append) 8957076 (submit) 9166516 (start) 10052116 (end)
Device Timeline (queue: 0x5d8082b7bad8): GEMM<4.1> [ns] = 8496499 (append) 8957076 (submit) 10052276 (start) 13385396 (end)
Device Timeline (queue: 0x5d8082b7bad8): zeCommandListAppendBarrier<5.1> [ns] = 8501125 (append) 8957076 (submit) 10052916 (start) 13387636 (end)
Device Timeline (queue: 0x5d8082b7bad8): zeCommandListAppendMemoryCopy(D2M)<6.1> [ns] = 8507201 (append) 8957076 (submit) 13387796 (start) 13725396 (end)
...
```
**Chrome Device Timeline** mode dumps timestamps for device activities per command queue to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool. Can't be used with **Chrome Kernel Timeline** and **Chrome Device Stages**.

**Chrome Kernel Timeline** mode dumps timestamps for device activities per kernel name to JSON format that can be opened in [chrome://tracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/) browser tool. Can't be used with **Chrome Device Timeline**.

**Chrome Device Stages** mode provides alternative view for device queue where each kernel invocation is divided into stages: "appended", "sumbitted" and "execution". Can't be used with **Chrome Device Timeline**.

**Conditional Collection** mode allows one to enable data collection for any target interval (by default collection will be disabled) using environment variable `PTI_ENABLE_COLLECTION`, e.g.:
```cpp
// Collection disabled
setenv("PTI_ENABLE_COLLECTION", "1", 1);
// Collection enabled
unsetenv("PTI_ENABLE_COLLECTION");
// Collection disabled
```
All the API calls and kernels, which submission happens while collection disabled interval, will be omitted from final results.

**Traced Level Zero APIs:** Call logging and host timing (`-c`, `-h`, `--chrome-call-logging`) cover every API of the Level Zero headers that the tool is built with: the callbacks are generated from them at build time (`gen_tracing_callbacks.py`). The 1.0 APIs are registered with `zelTracerSetPrologues/Epilogues`. Newer ones, e.g. `zeCommandListAppendLaunchKernelWithArguments`, which the SYCL Level Zero v2 adapter uses for kernel launches, are registered with the per-API `zelTracer*RegisterCallback` functions of the loader. These are looked up at run time, so APIs that the installed loader doesn't know are skipped. Driver extension functions that an application gets with `zeDriverGetExtensionFunctionAddress` (e.g. `zexCounterBasedEventCreate2`) are called directly, bypassing the loader, and are not traced.

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
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/tools/ze_tracer
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./ze_tracer [options] <target_application>
```
One may use [ze_gemm](../../samples/ze_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
./ze_tracer -c -h ../../../samples/ze_gemm/build/ze_gemm
./ze_tracer -c -h ../../../samples/dpc_gemm/build/dpc_gemm
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\tools\ze_tracer
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the tool:
```sh
ze_tracer.exe [options] <target_application>
```
One may use [ze_gemm](../../samples/ze_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
ze_tracer.exe -c -h ..\..\..\samples\ze_gemm\build\ze_gemm.exe
ze_tracer.exe -c -h ..\..\..\samples\dpc_gemm\build\dpc_gemm.exe
```
