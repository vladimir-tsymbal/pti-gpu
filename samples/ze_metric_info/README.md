# Level Zero Metrics Info
## Overview
This sample application queries the information about the hardware metrics for Intel(R) Processor Graphics, available to collect through Level Zero Metrics API.

Expected output should be the following (DN means the domain number, TBS and EBS are time or event based sampling correspondingly):
```
Target device: <GPU name>
Metric Group 0: D3 / TBS / RenderBasic (RenderBasic)
    Metric 0: D3 / TBS / RenderBasic / GpuTime (Time elapsed on the GPU during the measurement.) [UINT64, EVENT]
    Metric 1: D3 / TBS / RenderBasic / GpuCoreClocks (The total number of GPU core clocks elapsed during the measurement.) [UINT64, EVENT]
    Metric 2: D3 / TBS / RenderBasic / AvgGpuCoreFrequencyMHz (Average GPU Core Frequency in the measurement.) [UINT64, EVENT]
    Metric 3: D3 / TBS / RenderBasic / ResultUncertainty (Result uncertainty indicator) [UINT64, EVENT]
    ...
Metric Group 1: D3 / EBS / RenderBasic (RenderBasic)
    Metric 0: D3 / EBS / RenderBasic / GpuTime (Time elapsed on the GPU during the measurement.) [UINT64, EVENT]
    Metric 1: D3 / EBS / RenderBasic / GpuCoreClocks (The total number of GPU core clocks elapsed during the measurement.) [UINT64, EVENT]
    Metric 2: D3 / EBS / RenderBasic / AvgGpuCoreFrequencyMHz (Average GPU Core Frequency in the measurement.) [UINT64, EVENT]
    Metric 3: D3 / EBS / RenderBasic / ResultUncertainty (Result uncertainty indicator) [UINT64, EVENT]
...
```
To set target device and sub-device to collect metric info from one can specify `PTI_DEVICE_ID` and `PTI_SUB_DEVICE_ID` environment variables.

## Supported OS
- Linux
- Windows (*under development*)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)
- Metrics (OA) collection: tested with both kernel drivers

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)
- [Intel(R) Metrics Discovery Application Programming Interface](https://github.com/intel/metrics-discovery)
- [Metrics Library for Metrics Discovery API (Metrics Library for MD API)](https://github.com/intel/metrics-library)
- On Linux one may need to enable metrics collection for non-root users:
  ```sh
  echo 0 | sudo tee /proc/sys/dev/i915/perf_stream_paranoid  # i915 kernel driver (e.g. PVC)
  echo 0 | sudo tee /proc/sys/dev/xe/observation_paranoid    # xe kernel driver (e.g. BMG)
  ```

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/ze_metric_info
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the utility:
```sh
./ze_metric_info
```
Since Intel(R) Metrics Discovery Application Programming Interface library is loaded at runtime, one may need to set its path explicitly, e.g.:
```sh
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib ./ze_metric_info
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\ze_metric_info
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the application:
```sh
ze_metric_info.exe
```
Since Intel(R) Metrics Discovery Application Programming Interface and Metrics Library for Metrics Discovery API (Metrics Library for MD API) are loaded at runtime, one may need to set its path explicitly (see the output of cmake), e.g.:
```sh
set PATH=%PATH%;C:\Windows\system32\DriverStore\FileRepository\igdlh64.inf_amd64_d59561bc9241aaf5
ze_metric_info.exe
```