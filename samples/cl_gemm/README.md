# OpenCL(TM) GEMM
## Overview
This sample application performs general matrix multiplication using OpenCL(TM) CPU or GPU device, so it can be used as a target for OpenCL(TM) profiling and tracing tools.
```
OpenCL Matrix Multiplication (matrix size: 1024 x 1024, repeats 4 times)
Target device: <GPU name>
Matrix multiplication time: 0.00333904 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00333824 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00333824 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.003336 sec
Results are CORRECT with accuracy: 4.90573e-06
Total execution time: 0.0266174 sec
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
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/cl_gemm
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the application:
```sh
./cl_gemm [cpu|gpu] [matrix_size] [repeat_count]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_gemm
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the application:
```sh
cl_gemm.exe [cpu|gpu] [matrix_size] [repeats_count]
```