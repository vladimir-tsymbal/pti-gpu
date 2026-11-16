# GPU Query for OpenCL(TM)
## Overview
This sample is a simple LD_PRELOAD based tool that allows to collect such GPU hardware metrics as execution unit (EU) active, stall and idle ratios attributed to OpenCL(TM) kernels based on query metrics collection mode.

As a result, table like the following will be printed. For each kernel its call count, total time and metric values will be shown.
```
=== Device Metrics: ===

Total Execution Time (ns): 77356864
Total Kernel Time (ns): 29159061

    Kernel,       Calls,           Time (ns),        Time (%),        Average (ns),   EU Active (%),    EU Stall (%),     EU Idle (%)
      GEMM,           4,            29159061,          100.00,             7289765,           81.69,           16.38,            1.93
```
To set target device and sub-device to collect metrics from one can specify `PTI_DEVICE_ID` and `PTI_SUB_DEVICE_ID` environment variables.

## Supported OS
- Linux
- Windows (not validated)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver) and Intel(R) Arc(TM) B570 Graphics (xe kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)
- Metrics (OA) collection: tested with both kernel drivers

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [OpenCL(TM) ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)
- [Intel(R) Metrics Discovery Application Programming Interface](https://github.com/intel/metrics-discovery)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/cl_gpu_query
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./cl_gpu_query <target_application>
```
One may use [cl_gemm](../cl_gemm) as target application:
```sh
./cl_gpu_query ../../cl_gemm/build/cl_gemm gpu
```
Since Intel(R) Metrics Discovery Application Programming Interface library is loaded at runtime, one may need to set its path explicitly, e.g.:
```sh
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib ./cl_gpu_query ../../cl_gemm/build/cl_gemm gpu
```
On Linux one may need to enable metrics collection for non-root users:
```sh
echo 0 | sudo tee /proc/sys/dev/i915/perf_stream_paranoid  # i915 kernel driver (e.g. PVC)
echo 0 | sudo tee /proc/sys/dev/xe/observation_paranoid    # xe kernel driver (e.g. BMG)
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_gpu_query
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
cl_gpu_query.exe <target_application>
```
One may use [cl_gemm](../cl_gemm) as target application:
```sh
cl_gpu_query.exe ..\..\cl_gemm\build\cl_gemm.exe
```
Since Intel(R) Metrics Discovery Application Programming Interface library is loaded at runtime, one may need to set its path explicitly (see the output of cmake), e.g.:
```sh
set PATH=%PATH%;C:\Windows\system32\DriverStore\FileRepository\igdlh64.inf_amd64_d59561bc9241aaf5
cl_gpu_query.exe ..\..\cl_gemm\build\cl_gemm.exe
```