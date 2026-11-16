# Level Zero General Matrix Multiplication
## Overview
This sample application performs general matrix multiplication using Level Zero GPU device, so it can be used as a target for Level Zero profiling and tracing tools.
```
Level Zero Matrix Multiplication (matrix size: 1024 x 1024, repeats 4 times)
Target device: <GPU name>
Matrix multiplication time: 0.0033336 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00333472 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00333488 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.003336 sec
Results are CORRECT with accuracy: 4.90573e-06
Total execution time: 0.0264966 sec
```
To run application on target device and sub-device one can specify `PTI_DEVICE_ID` and `PTI_SUB_DEVICE_ID` environment variables.

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
cd <pti>/samples/ze_gemm
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the application:
```sh
./ze_gemm [matrix_size] [repeats_count]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\ze_gemm
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the application:
```sh
ze_gemm.exe [matrix_size] [repeats_count]
```