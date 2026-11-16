# DPC++ Info
## Overview
This sample application provides information on all platforms and devices available in DPC++ (similar to `clinfo` tool).

The following modes are implemented:
* Detailed information on each platform and device (`-a`):
    ```
      Number of  platforms                            3
      Platform Name                                   Intel(R) OpenCL
      Platform Vendor                                 Intel(R) Corporation
      Platform Profile                                FULL_PROFILE
      Platform Name                                   Intel(R) OpenCL Graphics
      Platform Vendor                                 Intel(R) Corporation
      Platform Profile                                FULL_PROFILE
      Platform Name                                   Intel(R) oneAPI Unified Runtime over Level-Zero
      Platform Vendor                                 Intel(R) Corporation
      Platform Profile                                FULL_PROFILE
      Platform Name                                   Intel(R) OpenCL
    Number of devices                                 1
      Device Name                                     <CPU name>
      Device Vendor                                   Intel(R) Corporation
      Device vendor ID                                0x8086
      Device Version                                  OpenCL 3.1 (Build 0)
      Driver Version                                  2026.22.10.0.01_160000
      Device SYCL Version                             OpenCL 3.1 (Build 0)
      Device type                                     CPU
      Device Available                                Yes
      Compiler Available                              Yes
      Linker Available                                Yes
      Max compute units                               96
      Max clock frequency                             2800MHz
    ...
    ```

* List of available devices and platforms (`-l`):
    ```
    Platform #0: Intel(R) OpenCL
     `-- Device #0: <CPU name>
    Platform #1: Intel(R) OpenCL Graphics
     `-- Device #0: <GPU name>
     `-- Device #1: <GPU name>
    Platform #2: Intel(R) oneAPI Unified Runtime over Level-Zero
     `-- Device #0: <GPU name>
     `-- Device #1: <GPU name>
    ```

## Supported OS
- Linux
- Windows (not validated)

## Tested Configurations
- Devices: lists every SYCL device; tested with Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver) and the OpenCL(TM) CPU device; other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [Intel(R) oneAPI Toolkit](https://www.intel.com/content/www/us/en/developer/tools/oneapi/oneapi-toolkit.html) (Intel(R) oneAPI DPC++/C++ Compiler, `icpx`)

## Build and Run
### Linux
Run the following commands to build the sample (make sure you have oneAPI DPC++ Compiler in `PATH` for building):
```sh
source <inteloneapi>/setvars.sh
cd <pti>/samples/dpc_info
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the utility:
```sh
./dpc_info [-l|-a]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample (make sure you have oneAPI DPC++ Compiler in `PATH` for building):
```sh
<inteloneapi>\setvars.bat
cd <pti>\samples\dpc_info
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
Use this command line to run the application:
```sh
cd Release
dpc_info.exe [-l|-a]
```