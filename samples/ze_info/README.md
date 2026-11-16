# Level Zero Info
## Overview
This sample application provides information on all platforms and devices available through oneAPI Level Zero (Level Zero), similar to `clinfo` tool.

* Detailed information on each driver and device (`-a`):
    ```
    Number of drivers                                 1
      Driver API Version                              1.6
      Driver Version                                  17007402

      Driver                                          0
    Number of devices                                 2
      Device Name                                     <GPU name>
      Device Type                                     GPU
      Vendor ID                                       0x8086
      Device ID                                       0xbd6
      Subdevice ID                                    0x0
      Core Clock Rate                                 1600MHz
      Maximum Memory Allocation Size                  65267564544 (60.79GiB)
      Maximum Hardware Contexts                       65536
      Maximum Command Queue Priority                  0
      Number Threads Per EU                           8
      Physical EU SIMD Width                          16
      Number EU Per SubSlice                          8
      Number SubSlices Per Slice                      56
      Number Slices                                   1
      Timer Resolution                                12500000clks
      ...
    ```
* List of available drivers and platforms (`-l`):
    ```
    Driver #0: API Version 1.6
    -- Device #0: <GPU name>
    -- Device #1: <GPU name>
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
cd <pti>/samples/ze_info
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the utility:
```sh
./ze_info [-l|-a]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\ze_info
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the application:
```sh
ze_info.exe
```