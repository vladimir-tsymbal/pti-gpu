# OpenCL(TM) GEMM with Manual Code Instrumentation
## Overview
This sample application performs general matrix multiplication using OpenCL(TM) GPU device and provides some statistics based on manual source code instrumentation with OpenCL built-in intrinsics.
```
OpenCL Matrix Multiplication (matrix size: 1024 x 1024, repeats 4 times)
Target device: <GPU name>
Matrix multiplication time: 0.00334192 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00334144 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00334112 sec
Results are CORRECT with accuracy: 4.90573e-06
Matrix multiplication time: 0.00334016 sec
Results are CORRECT with accuracy: 4.90573e-06
======== GEMM Instrumentation Results ========
Samples collected: 131072 (0.00% of bad samples)
Estimated average HW thread duration: ~ 547 us (547042 cycles)
Estimated number of HW threads per EU: 8
Estimated number of EUs per subslice: 8
Estimated number of subslices per dual subslice: 7
Estimated number of dual subslices per slice: 7
Estimated number of slices: 8
Estimated total number of EUs: 3136
Estimated total number of HW threads: 25088
==============================================
Total execution time: 0.04 sec
```
## Supported OS
- Linux
- Windows (not validated)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); other devices are not tested
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
cd <pti>/samples/cl_gemm_inst
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the application:
```sh
./cl_gemm_inst [matrix_size] [repeat_count]
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_gemm_inst
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the application:
```sh
cl_gemm_inst.exe [matrix_size] [repeats_count]