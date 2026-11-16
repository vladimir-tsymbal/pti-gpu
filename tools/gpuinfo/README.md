# GPU Information
## Overview
This sample application queries the information about the Intel(R) Processor Graphics device, that includes general GPU properties and hardware metrics, available to collect through [Intel(R) Metrics Discovery Application Programming Interface](https://github.com/intel/metrics-discovery).

This application could be used as the test to check if Intel(R) Metrics Discovery Application Programming Interface library is available on the system.
The following options are supported:
```
Usage: ./gpuinfo[.exe] <options>
Options:
--list [-l]      Print list of available devices
--info [-i]      Print general device info (default)
--metrics [-m]   Print list of available metrics
--help [-h]      Print this help message
--version        Print version
```

Expected output should be the following:
* In case of printing the list of available GPU devices (`-l`):
    ```
    Device 0:
    -- Subdevice 0: /dev/dri/renderD128
    ```
* In case of retrieving device information (`-i`) the following output is expected for all of the devices and subdevices:
    ```
    Device 0:
    -- Subdevice 0:
    ---- Name: /dev/dri/renderD128
    ---- GeometryPipeTotalCount: 5
    ---- GtCopyEngine2: true
    ---- GtCopyEngine1: true
    ---- GtCopyEngine0: true
    ---- GtCopyEngineMask: unknown
    ---- GtSqidi4: true
    ---- GtSqidi3: true
    ---- GtSqidi2: true
    ---- VectorEnginePerXeCoreCount: 8
    ---- GtXeCore7: true
    ---- GtXeCore5: true
    ---- GtXeCore3: true
    ---- GtL3Bank23: true
    ---- VectorEngineThreadsCount: 8
    ---- GpuTimestampFrequency: 19200000
    ---- MediaOABufferMinSize: 131072
    ---- GtXeCore2: true
    ---- GtL3Node4: true
    ---- GtSlice0: true
    ---- GtXeCore1: true
    ---- PciDeviceId: e20c
    ---- OABufferMaxSize: 134217728
    ...
    ```
* In case of retrieving metrics information (`-m`) the following output is expected for all of the devices and subdevices:
    ```
    Device 0:
    -- Subdevice 0:
    ---- Metric Group 0: OcclusionQueryStats
    ------ Metric Set 0: RenderedPixelsStats (Rendered Pixels Statistics)
    -------- Metric 0: OcclusionQueryStats / RenderedPixelsStats / PixelsRendered (Depth passed pixels) [UINT64]
    ------ Metric Set 1: RenderedFragmentsStats (Rendered Fragments Statistics)
    -------- Metric 0: OcclusionQueryStats / RenderedFragmentsStats / PixelsRendered (Depth passed fragments) [UINT64]
    ---- Metric Group 1: TimestampQuery
    ------ Metric Set 0: GPUTimestamp (GPU Timestamp)
    -------- Metric 0: TimestampQuery / GPUTimestamp / GpuDuration (GPU Duration) [UINT64]
    ---- Metric Group 2: PipelineStatistics
    ------ Metric Set 0: PipelineStats (Pipeline Statistics for OGL4)
    -------- Metric 0: PipelineStatistics / PipelineStats / IAVertices (Input vertices) [UINT64]
    -------- Metric 1: PipelineStatistics / PipelineStats / IAPrimitives (Input primitives) [UINT64]
    -------- Metric 2: PipelineStatistics / PipelineStats / VsInvocations (VS per vertex invocations) [UINT64]
    -------- Metric 3: PipelineStatistics / PipelineStats / HsInvocations (HS invocations) [UINT64]
    -------- Metric 4: PipelineStatistics / PipelineStats / DsInvocations (DS invocations) [UINT64]
    -------- Metric 5: PipelineStatistics / PipelineStats / GsInvocations (GS per triangle invocations) [UINT64]
    -------- Metric 6: PipelineStatistics / PipelineStats / GsPrimitives (Post-GS primitives) [UINT64]
    -------- Metric 7: PipelineStatistics / PipelineStats / ClipperInvocations (Clipper invocations) [UINT64]
    -------- Metric 8: PipelineStatistics / PipelineStats / ClipperPrimitives (Clipper primitives) [UINT64]
    ...
    ```
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
- [Intel(R) Metrics Discovery Application Programming Interface](https://github.com/intel/metrics-discovery)
- On Linux one may need to enable metrics collection for non-root users:
  ```sh
  echo 0 | sudo tee /proc/sys/dev/i915/perf_stream_paranoid  # i915 kernel driver (e.g. PVC)
  echo 0 | sudo tee /proc/sys/dev/xe/observation_paranoid    # xe kernel driver (e.g. BMG)
  ```

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/tools/gpuinfo
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the utility:
```sh
./gpuinfo <options>
```
Since Intel(R) Metrics Discovery Application Programming Interface library is loaded at runtime, one may need to set its path explicitly, e.g.:
```sh
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib ./gpuinfo
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\tools\gpuinfo
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
nmake
```
Use this command line to run the application:
```sh
gpuinfo.exe <options>
```
Since Intel(R) Metrics Discovery Application Programming Interface library is loaded at runtime, one may need to set its path explicitly (see the output of cmake), e.g.:
```sh
set PATH=%PATH%;C:\Windows\system32\DriverStore\FileRepository\igdlh64.inf_amd64_d59561bc9241aaf5
gpuinfo.exe
```