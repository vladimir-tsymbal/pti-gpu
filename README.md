# Profiling Tools Interfaces for GPU (PTI for GPU)

## About this fork

This fork is a maintained continuation of the PTI-GPU tools and samples that
upstream marked as no longer under active development. The PTI SDK and
`unitrace` are not part of this fork.

- Targets Linux with both the i915 and xe kernel drivers, and is tested on
  both (see [Tested Configurations](#tested-configurations)). The Windows
  build paths are kept, but Windows is not tested here.
- Tools: [onetrace](tools/onetrace/README.md), [oneprof](tools/oneprof/README.md),
  [ze_tracer](tools/ze_tracer/README.md), [cl_tracer](tools/cl_tracer/README.md),
  [gpuinfo](tools/gpuinfo/README.md), [instcount](tools/instcount/README.md),
  [memaccess](tools/memaccess/README.md), [sysmon](tools/sysmon/README.md).
- Also included: the [samples](#sample-tools--utilities), the profiling
  [chapters](#profiling-chapters), and the `tests/run.py` test driver.

## Overview

This repository describes the ways of collecting performance data for Intel(R)
Processor Graphics and provides a set of samples that help to start.

## License

Samples for Profiling Tools Interfaces for GPU (PTI for GPU) are distributed
under the MIT License.

You may obtain a copy of the License at <https://opensource.org/license/MIT>

## Supported OS

- Linux (i915 and xe kernel drivers): developed and tested
- Windows: build paths kept, not tested in this fork

## Supported Platforms

- Intel® Data Center GPU Max Series and Intel® Arc™ B-Series Graphics are
  tested in this fork (see below).
- Upstream also listed Intel® Processor Graphics Gen9 (formerly Skylake)
  and newer, Intel® Iris® Xe Graphics, Intel® Data Center GPU Flex Series
  and other Intel® Arc™ Graphics. These are not tested in this fork.

*Some samples may have higher hardware requirements.*

## Tested Configurations

| | i915 | xe |
|---|---|---|
| GPU | Intel® Data Center GPU Max 1550 | Intel® Arc™ B570 Graphics |
| OS, kernel | Ubuntu 24.04, 6.8 | Ubuntu 26.04, 7.0 |
| Kernel driver | i915 out-of-tree DKMS 1.25.2 | xe (in-tree) |
| Compute runtime | 25.18 | 26.05 |
| Level Zero loader (headers) | 1.24 (1.13) | 1.28 (1.15) |
| oneAPI | 2026.2 | 2026.2 |
| Metrics Discovery API, Metrics Library | 1.14.180, 1.0.196 | 1.16.199, 1.0.249 (built from source) |
| GT Pin | 5.0.0 | 5.0.0 |

Every test of `tests/run.py` passes on both, except the ones skipped by
design (see [Testing](#testing)). The metrics (OA) tests were run with
`perf_stream_paranoid=0` (i915, on a host with i915 DKMS 1.25.2.13, kernel
6.8.0-60 and oneAPI 2025.3) and `observation_paranoid=0` (xe). Exact package
versions and install steps: [SOFTWARE](SOFTWARE).

## Profiling Chapters

1. Runtime API Tracing
    - for [OpenCL(TM)](chapters/runtime_api_tracing/OpenCL.md)
    - for [oneAPI Level Zero (Level Zero)](chapters/runtime_api_tracing/LevelZero.md)
    - for [OpenMP*](chapters/runtime_api_tracing/OMPT.md)
2. Device Activity Tracing
    - for [OpenCL(TM)](chapters/device_activity_tracing/OpenCL.md)
    - for [oneAPI Level Zero (Level Zero)](chapters/device_activity_tracing/LevelZero.md)
    - for [SYCL/DPC++](chapters/device_activity_tracing/DPCXX.md)
3. Binary/Source Correlation
    - for [OpenCL(TM)](chapters/binary_source_correlation/OpenCL.md)
    - for [oneAPI Level Zero (Level Zero)](chapters/binary_source_correlation/LevelZero.md)
4. Metrics Collection
    - based on [oneAPI Level Zero (Level Zero) Metric API](chapters/metrics_collection/LevelZero.md)
    - based on [Intel(R) Metrics Discovery Application Programming Interface](chapters/metrics_collection/MetricsDiscoveryAPI.md)
    - based on [Performance Monitoring (PM) Register](chapters/metrics_collection/PerfMonReg.md)
5. Binary Instrumentation
    - based on [Graphics Technology Pin (GT Pin)](chapters/binary_instrumentation/GTPin.md)
    - based on [OpenCL(TM) built-in intrinsics](chapters/binary_instrumentation/OpenCLBuiltIn.md)
6. Code Annotation
    - based on [Instrumentation and Tracing Technology API (ITT API)](chapters/code_annotation/ITT.md)
7. System Management
    - for [oneAPI Level Zero (Level Zero)](chapters/system_management/LevelZero.md)

## Profiling & Debug Tools

- [onetrace](tools/onetrace/README.md) - host and device tracing tool for
  OpenCL(TM) and Level Zero backends (including DPC++ and OpenMP* GPU offload);
- [ze_tracer](tools/ze_tracer/README.md) - host and device tracing tool for
  Level Zero;
- [cl_tracer](tools/cl_tracer/README.md) - host and device tracing tool for
  OpenCL(TM);
- [oneprof](tools/oneprof/README.md) - GPU HW metrics collection tool;
- [gpuinfo](tools/gpuinfo/README.md) - provides basic information about the
  GPUs installed in a system, and the list of HW metrics one can collect
  for it;
- [instcount](tools/instcount/README.md) - prints GPU kernel assembly (GEN ISA)
  annotated by instruction execution count;
- [memaccess](tools/memaccess/README.md) - GT Pin based analysis of GPU kernel
  memory accesses;
- [sysmon](tools/sysmon/README.md) - Linux "top" like utility to monitor GPUs
  installed on a system;

## Sample Tools & Utilities

- tools for OpenCL(TM), DPC++ (with OpenCL(TM) backend) and OpenMP* GPU
  offload (with OpenCL(TM) backend):
  - [cl_hot_functions](samples/cl_hot_functions/README.md) - provides a list of
    hottest OpenCL(TM) API calls by backend (CPU and GPU);
  - [cl_hot_kernels](samples/cl_hot_kernels/README.md) - provides a list of
    hottest OpenCL(TM) kernels by backend (CPU and GPU);
  - [cl_debug_info](samples/cl_debug_info/README.md) - prints source and
    assembly (GEN ISA) for kernels on GPU;
  - [cl_gpu_metrics](samples/cl_gpu_metrics/README.md) - provides a list of
    hottest OpenCL(TM) GPU kernels along with percent of cycles it was active,
    stall and idle (based on continuous metrics collection mode);
  - [cl_gpu_query](samples/cl_gpu_query/README.md) - provides a list of hottest
    OpenCL(TM) GPU kernels along with percent of cycles it was active, stall
    and idle (based on query metrics collection mode);
- tools for Level Zero, DPC++ (with Level Zero backend) and OpenMP* GPU
  offload (with Level Zero backend):
  - [ze_hot_functions](samples/ze_hot_functions/README.md) - provides a list of
    hottest Level Zero API calls;
  - [ze_hot_kernels](samples/ze_hot_kernels/README.md) - provides a list of
    hottest Level Zero kernels;
  - [ze_debug_info](samples/ze_debug_info/README.md) - prints source and
    assembly (GEN ISA) for kernels on GPU;
  - [ze_metric_query](samples/ze_metric_query/README.md) - provides a list of
    hottest Level Zero GPU kernels along with percent of cycles it was active,
    stall and idle (metrics are collected in *query* mode);
  - [ze_metric_streamer](samples/ze_metric_streamer/README.md) - provides a
    list of hottest Level Zero GPU kernels along with percent of cycles it was
    active, stall and idle (metrics are collected in *streamer* mode);
- tools for OpenMP*:
  - [omp_hot_regions](samples/omp_hot_regions/README.md) - provides a list of
    hottest parallel (for CPU) and target (for GPU) OpenMP* regions;
- utilities:
  - [dpc_info](samples/dpc_info/README.md) - prints information on available
    platforms and devices in DPC++;
  - [ze_info](samples/ze_info/README.md) - prints information on available
    platforms and devices in Level Zero;
  - [ze_metric_info](samples/ze_metric_info/README.md) - prints the list of HW
    metrics one can collect with the help of Level Zero;
  - [gpu_perfmon_set](samples/gpu_perfmon_set/README.md) - allows to choose
    HW metric for collection in EU PerfMon register;

## Prerequisites

- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- On Linux one has to be a part of the `render` user group (the group of
  `/dev/dri/renderD*`) to do any computations on Intel(R) Processor Graphics.
  [gpu_perfmon_set](samples/gpu_perfmon_set/README.md) may also open
  `/dev/dri/card*`, which belongs to the `video` group:

  ```sh
  sudo usermod -a -G render <username>
  sudo usermod -a -G video <username>  # for /dev/dri/card*
  ```

- OpenCL(TM) [ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
  and [Headers](https://github.com/KhronosGroup/OpenCL-Headers)
  - to use non-standard path to OpenCL ICD library one may add it into
    `LD_LIBRARY_PATH`:

    ```sh
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<path-to-opencl>
    ```

- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)
- [Intel(R) Metrics Discovery Application Programming Interface](https://github.com/intel/metrics-discovery)
  - one may need to install `libdrm-dev` package to build the library
    from sources
  - one may need to allow metrics collection for non-root users
    - if using the i915 kernel module (e.g. PVC):

      ```sh
      echo "0" | sudo tee /proc/sys/dev/i915/perf_stream_paranoid
      ```

    - if using the xe kernel module (e.g. BMG):

      ```sh
      echo "0" | sudo tee /proc/sys/dev/xe/observation_paranoid
      ```

- [Metrics Library for Metrics Discovery API (Metrics Library for MD API)](https://github.com/intel/metrics-library)
- [Graphics Technology Pin (GT Pin)](https://www.intel.com/content/www/us/en/developer/articles/tool/gtpin.html)
- [Intel(R) oneAPI Toolkit](https://www.intel.com/content/www/us/en/developer/tools/oneapi/oneapi-toolkit.html) (Intel(R) oneAPI DPC++/C++ Compiler `icpx`, for the DPC++ and OpenMP samples)
- [libdrm](https://dri.freedesktop.org/libdrm/)
  - on Ubuntu one may perform:

    ```sh
    sudo apt-get install libdrm-dev
    ```

More information of what is needed for particular sample can be found on
sample description page.

## Build and Run

### Building everything

The top-level CMake project builds every tool and sample, each as a
separate CMake sub-build in `build/tools/<name>` and `build/samples/<name>`:

```sh
source /opt/intel/oneapi/setvars.sh  # needed for the DPC++/OpenMP samples
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix <install_dir>  # installs the tools into <install_dir>/bin
```

Options:

| Option | Default | Description |
|---|---|---|
| `PTI_GPU_BUILD_TOOLS` | `ON` | Build the tools |
| `PTI_GPU_BUILD_SAMPLES` | `ON` | Build the samples |
| `PTI_GPU_TOOLS` | all | List of tools to build, e.g. `"onetrace;sysmon"` |
| `PTI_GPU_SAMPLES` | all | List of samples to build |
| `PTI_GPU_SKIP` | empty | List of tools or samples to skip, e.g. `"instcount;memaccess"` |
| `BUILD_TESTING` | `OFF` | Add a `legacy` ctest that runs `tests/run.py` |

Samples that need `icpx` (DPC++ and OpenMP) are skipped automatically if the
compiler is not found, and the `*_debug_info` samples are skipped if the IGA
library (`libiga64.so`) is not found (`oneprof` is then built without the
instruction disassembly of `--stall-sampling`). `instcount` and `memaccess`
download GT Pin at configure time.

Per-tool packages can be created with CPack, e.g.:

```sh
cd build && cpack -G TGZ  # or DEB / RPM
```

### Building a single sample or tool

In general, to build samples one need to perform the following steps
(specific instructions for particular sample can be found on sample
description page):

```sh
cd <pti_root>/samples/<sample_root>
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

To point out to specific headers and libraries one may use
`-DCMAKE_INCLUDE_PATH` and `-DCMAKE_LIBRARY_PATH` options
correspondingly, e.g.:

```sh
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INCLUDE_PATH=/tmp/level_zero/include \
  -DCMAKE_LIBRARY_PATH=/tmp/level_zero/lib \
  ..
```

Run instructions may vary from sample to sample significantly, so they are
provided on particular sample description page.

## Testing

`tests/run.py` builds and runs the tests of every sample and tool. The
DPC++ and OpenMP tests need the oneAPI environment, and the metrics tests
need the Metrics Discovery API library (`LD_LIBRARY_PATH` if it is in
`/usr/local/lib`):

```sh
source /opt/intel/oneapi/setvars.sh
LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH python3 <pti_root>/tests/run.py
```

Options:

| Option | Description |
|---|---|
| `-s <regex>` | Run only the samples and tools whose name matches, e.g. `-s cl_hot_functions` or `-s ze` |
| `-d` | Build in Debug mode instead of Release |
| `--skip-oa` | Skip every test that opens GPU metric (OA) streams (same as `PTI_TEST_SKIP_OA=1`) |
| `--log-dir <dir>` | Write `stderr.log`, `results.json` and `junit.xml` to `<dir>` (default: the current directory; also `PTI_TEST_LOG_DIR`) |
| `--timeout <seconds>` | Time limit per test (default 600, `0` = no limit) |
| `-c` | Remove the `build` directories the tests created, and the log and report files |

Each test is reported as PASSED, FAILED or SKIPPED, followed by a summary
with the reason for every failed or skipped test. The error output of the
failed tests is in `stderr.log`. `results.json` lists every test with its
status, time and reason, and `junit.xml` holds the same results in JUnit
format for CI. A test is SKIPPED when something it needs is missing on the
host, e.g.:

- metric (OA) streams are not allowed: `--skip-oa`, or
  `perf_stream_paranoid` / `observation_paranoid` is not `0` for non-root
  users (see [Prerequisites](#prerequisites));
- the Metrics Discovery API library (`libigdmd.so`) is not found;
- `icpx` is not found (DPC++ and OpenMP tests);
- GT Pin is not available (download failed, or `GTPIN_PATH` is wrong);
- the OpenCL runtime doesn't provide the Intel tracing extension
  (`clCreateTracingHandleINTEL`).

On the [tested configurations](#tested-configurations) a run with
`--skip-oa` reports 91 passed, 0 failed and 18 skipped (the metrics tests),
and the metrics tests pass when OA access is allowed.

**Tested software versions one may find in [SOFTWARE](SOFTWARE) file.**

**(*) Other names and brands may be claimed as property of others.**
