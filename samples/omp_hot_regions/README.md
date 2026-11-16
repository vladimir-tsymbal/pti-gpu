# OpenMP* Hot Regions
## Overview
This sample is a simple tool that allows to collect specific OpenMP* regions within an application along with their total execution time and call count. The tool is designed to work with `omp parallel` and `omp target` region types.

As a result, table like the following will be printed.
```
=== OpenMP Timing Results: ===

Total Execution Time (ns): 115239664
Total Region Time (ns): 22040135

           Region ID,         Region Type,       Calls, Transferred (bytes),           Time (ns),  Time (%),        Average (ns),            Min (ns),            Max (ns)
             4205481,              Target,           4,                   0,            22040135,    100.00,             5510033,             5302608,             5868359
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
cd <pti>/samples/omp_hot_regions
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
OMP_TOOL_LIBRARIES=./libomp_hot_regions.so <target_application>
```
One may use [omp_gemm](../omp_gemm) as target application:
```sh
OMP_TOOL_LIBRARIES=./libomp_hot_regions.so ../../omp_gemm/build/omp_gemm
```
### Windows (manual build)
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample (make sure you have Intel(R) C++ Compiler in `PATH` for building):
```sh
<inteloneapi>\setvars.sh
cd <pti>\samples\omp_hot_regions
mkdir build
cd build
icx.exe ../tool.cc /Qnextgen /Qopenmp -I../../../utils -o omp_hot_regions.dll /LD
```
Use this command line to run the tool:
```sh
set OMP_TOOL_LIBRARIES=omp_hot_regions.dll
<target_application>
```
One may use [omp_gemm](../omp_gemm) as target application:
```sh
set OMP_TOOL_LIBRARIES=omp_hot_regions.dll
..\..\omp_gemm\build\omp_gemm.exe
```