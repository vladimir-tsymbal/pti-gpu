# GPU Memory Access Analysis

## Overview

This tool collects information about memory accesses using binary instrumentation. It provides next information: 
 * Access pattern (stride distribution)
 * Cache lines transferred between cache and compute units
 * Instruction count, SIMD lanes count for memory accesses
 * Percent cache line not aligned accesses
 * Addresses sample

As a result, list of memory access instructions with collected metrics will be printed for each kernel.
```console
=== GEMM (runs 2 times)
--------------------------------------------------------------------------------
0x000288 :         send.ugm (32|M0)         r21      r16     null:0  0x0            0x08200580           {$4} // wr:4+0, rd:2; load.ugm.d32.a64
 (no source info)
  * SIMD32 ExecSize_32 4 bytes X1 Scatter A64 Read
  * Instruction executed: 67108864
  * SIMD lanes executed: 2147308781
  * Cache line transferred: 67103656 ( 4294633984 bytes)
      199.999 % used/transferred ratio
  * Cache line not aligned: 93.75 % (62914881)
  * Stride distribution:
      100% (2080374784) -> stride: 0 bytes (0 units)
  * Access addresses sample (SIMD32):
      Addr#  0   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr#  4   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr#  8   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr# 12   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr# 16   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr# 20   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr# 24   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc
      Addr# 28   0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc 0xff00ffffffdfdffc

--------------------------------------------------------------------------------
...
```

The following capabilities are available:
```console
--kernel-run                   Kernel run to profile
--stride-min                   Minimal detected stride (bytes)
--stride-num                   Number of collected strides (buckets)
--stride-step                  Stride step (bytes)
--json-output                  Print results in JSON format
```

## Supported OS

- Ubuntu 24.04 with the i915 kernel driver on Intel(R) Data Center GPU Max 1550, and Ubuntu 26.04 with the xe kernel driver on Intel(R) Arc(TM) B570 Graphics (last check: GT Pin 5.0.0, compute-runtime 25.18, 26.05 and 26.40; versions in [SOFTWARE](../../SOFTWARE))
- Ubuntu 24.04 (kernel 7.0) with the xe kernel driver on Intel(R) Arc(TM) Pro B70 Graphics: SYCL (DPC++) kernels are not instrumented (GT Pin 5.0.0 finds no kernels); on an Arrow Lake iGPU (i915), the applications fail under GT Pin 5.0.0
- Windows 11 (last check: GTPin v4.0, driver 31.0.101.4887)

## Prerequisites

- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Graphics Technology Pin (GT Pin)](https://www.intel.com/content/www/us/en/developer/articles/tool/gtpin.html)

## Build and Run

### Linux

CMake downloads GT Pin 5.0.0 automatically. If the download fails (for example, behind a proxy), get the package from the [GT Pin 5.0.0 download page](https://www.intel.com/content/www/us/en/download/730596/924057/gtpin.html), unpack it and pass `-DGTPIN_PATH=<gtpin>`, the directory that contains `Lib` and `Include`. Use `-DGTPIN_LINK=<url>` to download another package, and `-DGTPIN_DOWNLOAD_INACTIVITY_TIMEOUT=<seconds>` (default 60) or `-DGTPIN_DOWNLOAD_TIMEOUT=<seconds>` (default 900) to change when a stalled download gives up.

Run the following commands to build the sample:

```sh
cd <pti>/tools/memaccess
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release [-DGTPIN_PATH=<gtpin>] ..
make
```

Use this command line to run the tool:

```sh
./memaccess <target_application>
```

One may use [cl_gemm](../../samples/cl_gemm), [ze_gemm](../../samples/ze_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application:

```sh
./memaccess ../../../samples/cl_gemm/build/cl_gemm
./memaccess ../../../samples/ze_gemm/build/ze_gemm
./memaccess ../../../samples/dpc_gemm/build/dpc_gemm
```

### Windows

Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:

```sh
cd <pti>\samples\memaccess
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
nmake
```

Use this command line to run the tool:

```sh
set PATH=%PATH%;<gtpin>\Profilers\Lib\intel64
memaccess.exe <target_application>
```

CMake unpacks GTPin into "_deps\gtpin_package-src\Profilers":

```sh
set PATH=%PATH%;_deps\gtpin_package-src\Profilers\Lib\intel64
memaccess.exe <target_application>
```

One may use [cl_gemm](../../samples/cl_gemm), [ze_gemm](../../samples/ze_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application:

```sh
set PATH=%PATH%;<gtpin>\Profilers\Lib\intel64
memaccess.exe ..\..\cl_gemm\build\cl_gemm.exe
memaccess.exe ..\..\ze_gemm\build\ze_gemm.exe
memaccess.exe ..\..\dpc_gemm\build\dpc_gemm.exe
```

**Note**: to build this sample one may need to generate \*.lib file from IGA \*.dll (see [here](https://stackoverflow.com/questions/9946322/how-to-generate-an-import-library-lib-file-from-a-dll) for details) and provide the path to this \*.lib to cmake with `-DCMAKE_LIBRARY_PATH`.

Also one may need to add an actual path to IGA *.dll into PATH before sample run, e.g.:

```sh
set PATH=%PATH%;<gtpin>\Profilers\Lib\intel64
set PATH=%PATH%;<iga_dll_path>
memaccess.exe ..\..\cl_gemm\build\cl_gemm.exe
```
