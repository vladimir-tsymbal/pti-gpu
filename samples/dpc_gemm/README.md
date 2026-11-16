# DPC++ GEMM
## Overview
This sample application performs general matrix multiplication using DPC++ on CPU or GPU, so it can be used as a target for OpenCL(TM) and Level Zero profiling and tracing tools.
```
DPC++ Matrix Multiplication (matrix size: 1024 x 1024, repeats 4 times)
Target device: <GPU name>
Matrix multiplication time: 0.0033416 sec
Results are CORRECT with accuracy: 4.88761e-06
Matrix multiplication time: 0.00334048 sec
Results are CORRECT with accuracy: 4.88761e-06
Matrix multiplication time: 0.00333344 sec
Results are CORRECT with accuracy: 4.88761e-06
Matrix multiplication time: 0.00333424 sec
Results are CORRECT with accuracy: 4.88761e-06
Total execution time: 0.0299407 sec
```
## Supported OS
- Linux
- Windows (not validated)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver) and Intel(R) Arc(TM) B570 Graphics (xe kernel driver); on CPU, with the OpenCL(TM) CPU device of oneAPI 2026.2; other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [Intel(R) oneAPI Toolkit](https://www.intel.com/content/www/us/en/developer/tools/oneapi/oneapi-toolkit.html) (Intel(R) oneAPI DPC++/C++ Compiler, `icpx`)

## Build and Run
### Linux
Run the following commands to build the sample ((make sure you have oneAPI DPC++ Compiler in `PATH` for building)):
```sh
source <inteloneapi>/setvars.sh
cd <pti>/samples/dpc_gemm
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the application:
```sh
./dpc_gemm [cpu|gpu] [matrix_size] [repeat_count]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample (make sure you have oneAPI DPC++ Compiler in `PATH` for building):
```sh
<inteloneapi>\setvars.bat
cd <pti>\samples\dpc_gemm
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
Use this command line to run the application:
```sh
cd Release
dpc_gemm.exe [cpu|gpu] [matrix_size] [repeats_count]
```