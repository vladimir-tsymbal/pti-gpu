# GPU PerfMon Set
## Overview
This sample utility allows to tune Intel(R) Processor Graphics execution unit (EU) performance monitoring register to collect hardware events of some type.

The utility has a single parameter that contains the value one would like to set for events collection. Actual values are described in Programmer's Reference Manual for Intel(R) Processor Graphics (Kaby Lake), [Volume 14: Observability](https://cdrdv2.intel.com/v1/dl/getContent/723971), e.g.:
```sh
./gpu_perfmon_set 5 # EU Stall Collection
```
On start the utility will configure the driver, and GPU will collect desired events until user press ENTER button. Expected output should be the following:
```
Configuration with the given GUID is already added
GPU PefMon configuration is completed
Press ENTER to deconfigure the driver...
```
To read the collected values for the event one should use binary instrumentation techniques or inline assembly.

The utility works with both Linux kernel GPU drivers and picks the one in use at run time: i915 (i915 perf interface) and xe (xe observation interface, needs kernel headers from Linux 6.11 or newer at build time). Opening the metrics stream needs root, or `perf_stream_paranoid` (i915) / `observation_paranoid` (xe) set to 0:
```sh
echo 0 | sudo tee /proc/sys/dev/i915/perf_stream_paranoid  # i915 kernel driver (e.g. PVC)
echo 0 | sudo tee /proc/sys/dev/xe/observation_paranoid    # xe kernel driver (e.g. BMG)
```

## Supported OS
- Linux

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)
- Metrics (OA) collection: tested with both kernel drivers: the configuration is added and the stream opens. Whether the PM register counts the expected events on these GPUs is not verified

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [libdrm](https://dri.freedesktop.org/libdrm/)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/gpu_perfmon_set
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the utility:
```sh
./gpu_perfmon_set <perf_mon_reg_value>
```