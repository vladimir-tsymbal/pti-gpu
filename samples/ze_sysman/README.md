# Level Zero System Management
## Overview
This sample application provides basic static and dynamic information for GPU device:

Output should look like the following:
```
Device: <GPU name>
-- Subdevice Count: 2
-- Driver Version: unknown
-- PCI Bus: 0000:9a:00.0
-- Memory Modules: 2
---- [0] Module Capacity (MB): 65536
---- [1] Module Capacity (MB): 65536
-- Frequency Domains: 2
---- [0] Clock EU Freq Range (MHz): 200 - 1600 (changeable)
---- [0] Current Clock EU Freq (MHz): 1600
---- [1] Clock EU Freq Range (MHz): 200 - 1600 (changeable)
---- [1] Current Clock EU Freq (MHz): 0
Need to be root to see temperature
```

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
cd <pti>/samples/ze_sysman
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the utility:
```sh
./ze_sysman
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\ze_sysman
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the application:
```sh
ze_sysman.exe
```