# OpenMP* GEMM
## Overview
This sample application performs general matrix multiplication using OpenMP* on CPU or GPU, so it can be used as a target for OpenMP* profiling and tracing tools.
```
OpenMP Matrix Multiplication (matrix size: 1024 x 1024, repeats 4 times)
Target device: GPU
Matrix multiplication time: 0.0100588 sec
Results are CORRECT with accuracy: 4.88761e-06
Matrix multiplication time: 0.00634052 sec
Results are CORRECT with accuracy: 4.88761e-06
Matrix multiplication time: 0.00627159 sec
Results are CORRECT with accuracy: 4.88761e-06
Matrix multiplication time: 0.00617921 sec
Results are CORRECT with accuracy: 4.88761e-06
Total execution time: 0.0289379 sec
```
## Supported OS
- Linux
- Windows (*under development*)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); on the host CPU; other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [Intel(R) oneAPI Toolkit](https://www.intel.com/content/www/us/en/developer/tools/oneapi/oneapi-toolkit.html) (Intel(R) oneAPI DPC++/C++ Compiler, `icpx`)

## Build and Run
### Linux
Run the following commands to build the sample (make sure you have Intel(R) C++ Compiler in `PATH` for building):
```sh
source <inteloneapi>/setvars.sh
cd <pti>/samples/omp_gemm
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the application:
```sh
./omp_gemm [cpu|gpu] [matrix_size] [repeat_count]
```
### Windows (manual build)
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample (make sure you have Intel(R) C++ Compiler in `PATH` for building):
```sh
<inteloneapi>\setvars.bat
cd <pti>\samples\omp_gemm
mkdir build
cd build
icx.exe ../main.cc /Qopenmp /Qopenmp-targets=spir64 -I../../../utils -o omp_gemm.exe
```
Use this command line to run the application:
```sh
omp_gemm.exe [cpu|gpu] [matrix_size] [repeats_count]
```