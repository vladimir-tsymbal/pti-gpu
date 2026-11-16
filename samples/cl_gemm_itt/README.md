# OpenCL(TM) GEMM with Code Annotation
## Overview
This sample application performs general matrix multiplication using OpenCL(TM) CPU or GPU device. Its code is annotated with Instrumentation and Tracing Technology API (ITT API) that allows to highligh its regions-of-interest while using Intel(R) VTune(TM) Analyzer.
```
OpenCL Matrix Multiplication (matrix size: 1024 x 1024, repeats 4 times)
Target device: <GPU name>
Matrix multiplication time: 0.0033424 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.0033392 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00333808 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00333712 sec
Results are CORRECT with accuracy: 4.90573e-06
Total execution time: 0.0261914 sec
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
cd <pti>/samples/cl_gemm_itt
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the application:
```sh
./cl_gemm_itt [cpu|gpu] [matrix_size] [repeat_count]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_gemm_itt
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the application:
```sh
cl_gemm_itt.exe [cpu|gpu] [matrix_size] [repeats_count]
```

Use [Intel(R) VTune(TM) Analyzer](https://www.intel.com/content/www/us/en/developer/tools/oneapi/vtune-profiler.html) to profile this application in order to look into its regions-of-interest.