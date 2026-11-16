# System Monitoring Utility
## Overview
This utility provides basic static and dynamic information about the GPU, including the list of processes that uses this GPU at the moment - similar to Linux `top`.

The following options are supported:
```
Usage: ./sysmon [options]
Options:
--processes [-p]    Print short device information and running processes (default)
--list [-l]         Print list of devices and subdevices
--details [-d]      Print detailed information for all of the devices and subdevices
--help [-h]         Print help message
--version           Print version
```

**Processes** mode dumps short information about all the available GPUs and running processes (default mode):
```
=====================================================================================
GPU 0: <GPU name>    PCI Bus: 0000:9a:00.0
Vendor: Intel(R) Corporation    Driver Version: 1.3.33578    Subdevices: 0
EU Count: 448    Threads Per EU: 8    EU SIMD Width: 16    Total Memory(MB): 62244.0
Core Frequency(MHz): 1600.0 of 1600.0    Core Temperature(C): unknown
=====================================================================================
Running Processes: 1
     PID,  Device Memory Used(MB),  Shared Memory Used(MB),  GPU Engines, Executable
  237416,                     6.3,                     0.0,      UNKNOWN, ./sysmon
=====================================================================================
GPU 1: <GPU name>    PCI Bus: 0000:9a:00.0
Vendor: Intel(R) Corporation    Driver Version: 1.3.33578    Subdevices: 0
EU Count: 448    Threads Per EU: 8    EU SIMD Width: 16    Total Memory(MB): 62244.0
Core Frequency(MHz): 1600.0 of 1600.0    Core Temperature(C): unknown
=====================================================================================
Running Processes: 1
     PID,  Device Memory Used(MB),  Shared Memory Used(MB),  GPU Engines, Executable
  237416,                     6.3,                     0.0,      UNKNOWN, ./sysmon
```
*Note*, that the list of processes may be unavailable for integrated graphics. Core temperature is available only for *root* user.

**Device List** mode prints the list of devices and subdevices on the system:
```
Driver #0: API Version 1.6
-- Device #0: <GPU name>
-- Device #1: <GPU name>
```

**Details** mode dumps all available information for all devices and subdevices:
```
=====================================================================================
GPU 0
=====================================================================================
Name,                                   <GPU name>
Vendor ID,                              8086
Device ID,                              bd6
Core Clock Rate(MHz),                   1600
Number Of Slices,                       1
Number Of Subslices Per Slice,          56
Number Of EU Per Subslice,              8
Number Of Threads Per EU,               8
Total EU Count,                         448
Physical EU SIMD Width,                 16
Kernel Timestamp Valid Bits,            32
Max Command Queue Priority,             0
Timer Resolution(clks),                 12500000
Timestamp Valid Bits,                   36
Max Hardware Contexts,                  65536
Max Memory Allocation Size(MB),         62244.0
Max Total Group Size,                   1024
Max Group Size X,                       1024
Max Group Size Y,                       1024
Max Group Size Z,                       1024
Max Group Count X,                      4294967295
Max Group Count Y,                      4294967295
Max Group Count Z,                      4294967295
Max Shared Local Memory(KB),            128.0
Subgroup Sizes Supported,               16;32
SPIR-V Version Supported,               1.3
Device Module Flags,                    fp16;fp64;int64_atomics;dp4a
FP16 Denormals,                         yes
FP16 Infinity And NaN,                  yes
FP16 Round To Nearest Even,             yes
...
```

## Supported OS
- Linux

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver) and Intel(R) Arc(TM) B570 Graphics (xe kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero) and its development headers
  (`level_zero/ze_api.h`, `level_zero/zes_api.h`; e.g. the `libze-dev` package). Headers or a loader in
  another location can be passed with `-DCMAKE_INCLUDE_PATH=<dir>` and `-DCMAKE_LIBRARY_PATH=<dir>`.
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/tools/sysmon
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
sysmon is self-contained: the `tools/sysmon` directory can also be built outside the PTI repository. In that
case pass the version reported by `--version` with `-DSYSMON_VERSION=<version>`.
Use this command line to run the utility:
```sh
./sysmon
```
To run this utility in `top` like mode, one can use the following command:
```sh
watch -n 1 ./sysmon # Call sysmon every 1 second
```