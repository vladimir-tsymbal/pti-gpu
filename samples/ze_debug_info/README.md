# Level Zero Debug Info
## Overview
This sample is a simple LD_PRELOAD based tool that allows to collect oneAPI Level Zero kernel sources, binaries and symbols to correlate them. This tool works for Intel(R) Processor Graphics only.

As a result, listing like the following will be printed for each kernel (once per kernel creation):
```
===== Kernel: GEMM =====
=== File: Unknown ===
                [0x800000100430]         illegal
=== File: gemm.cl ===
                [0x8000001001F0] (W)     mul (16|M0)              acc0.0<1>:d   r10.0<1;1,0>:d    r5.4<0;1,0>:uw   {Compacted,I@2}
                [0x8000001001F8]         macl (16|M0)             r12.0<1>:d    r10.0<1;1,0>:d    r5.2<0;1,0>:d    {Compacted}
                [0x800000100200] (W)     mul (16|M16)             acc0.0<1>:d   r11.0<1;1,0>:d    r5.4<0;1,0>:uw   {Compacted}
[    1] __kernel void GEMM(__global float* a, __global float* b,
                [0x800000100000] (W)     mov (16|M0)              r127.0<1>:ud  0x0:ud
                [0x800000100010] (W)     and (1|M0)               r127.2<1>:ud  r0.0<0;1,0>:ud    0xFFFFFFC0:ud
                [0x800000100020] (W)     and (1|M0)               r127.0<1>:uw  r0.4<0;1,0>:uw    0xFF:uw
                [0x800000100030] (W)     add (1|M0)               r127.2<1>:ud  r127.2<0;1,0>:ud  0x60:ud              {I@2}
                [0x800000100040] (W)     add (1|M0)               r127.2<1>:ud  r127.2<0;1,0>:ud  0x0:ud              {I@1}
                [0x800000100050] (W)     mad (1|M0)               r127.0<1>:ud  r127.2<0;0>:ud    r127.0<0;0>:uw    0xC0:uw              {I@1}
                [0x800000100060] (W)     send.ugm (1|M0)          r1       r127    null:0  0xFF000000            0x6228E500           {A@1,$0} // wr:1+0, rd:2; load.ugm.d32x32t.a32.ca.ca.bti[255]
                [0x800000100070] (W)     add (1|M0)               r126.0<1>:ud  r127.0<0;1,0>:ud  0x80:uw
                [0x800000100080] (W)     send.ugm (1|M0)          r3       r126    null:0  0xFF000000            0x6218D500           {A@1,$1} // wr:1+0, rd:1; load.ugm.d32x16t.a32.ca.ca.bti[255]
                [0x800000100090] (W)     mov (1|M0)               null<1>:ud    0x30433:ud                              {$1.src}
                [0x8000001000A0] (W)     mov (1|M0)               null<1>:ud    r127.0<0;1,0>:ud
                [0x8000001000B0] (W)     mov (1|M0)               null<1>:ud    r126.0<0;1,0>:ud
                [0x8000001000C0]         sync.nop                             null                             {I@1}
                [0x8000001000D0]         sync.allrd                           ($0)
                [0x8000001000E0]         nop
                [0x8000001000F0]         nop
                [0x800000100100] (W)     and (1|M0)               r127.0<1>:ud  r0.0<0;1,0>:ud    0xFFFFFFC0:ud
                [0x800000100110] (W)     add (1|M0)               r127.0<1>:ud  r127.0<0;1,0>:ud  0x0:ud              {I@1}
                [0x800000100120] (W)     send.ugm (1|M0)          r4       r127    null:0  0xFF000000            0x6218D500           {I@1,$2} // wr:1+0, rd:1; load.ugm.d32x16t.a32.ca.ca.bti[255]
                [0x800000100130] (W)     add (1|M0)               r126.0<1>:ud  r127.0<0;1,0>:ud  0x40:uw
                [0x800000100140] (W)     send.ugm (1|M0)          r5       r126    null:0  0xFF000000            0x6218C500           {I@1,$3} // wr:1+0, rd:1; load.ugm.d32x8t.a32.ca.ca.bti[255]
                [0x800000100150] (W)     mov (1|M0)               null<1>:ud    0x30433:ud                              {$3.src}
                [0x800000100160] (W)     mov (1|M0)               null<1>:ud    r127.0<0;1,0>:ud                 {Compacted}
                [0x800000100168] (W)     mov (1|M0)               null<1>:ud    r126.0<0;1,0>:ud                 {Compacted}
                [0x800000100170]         sync.nop                             null                             {Compacted,I@1}
                [0x800000100178]         sync.allrd                           ($2)                 {Compacted}
                [0x800000100180] (W)     or (1|M0)                cr0.0<1>:ud   cr0.0<0;1,0>:ud   0x4C0:uw              {Compacted,A@1}
                [0x800000100188] (W)     mul (1|M0)               acc0.0<1>:d   r4.8<0;1,0>:d     r0.2<0;1,0>:uw   {Compacted,A@1,$2.dst}
                [0x800000100190] (W)     macl (1|M0)              r3.0<1>:d     r4.8<0;1,0>:d     r0.1<0;1,0>:d    {Compacted,$1.dst}
                [0x800000100198] (W)     mul (1|M0)               acc0.0<1>:d   r4.9<0;1,0>:d     r0.12<0;1,0>:uw
                [0x8000001001A8] (W)     macl (1|M0)              r9.0<1>:d     r4.9<0;1,0>:d     r0.6<0;1,0>:d
[    2]                     __global float* c, int size) {
...
```
**Note:** to collect debug information for DPC++ application one need to compile it with `-gline-tables-only` flag.

## Supported OS
- Linux
- Windows (*under development*)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)
- Intel(R) Processor Graphics Assembler (IGA) library `libiga64.so`, part of the [Intel(R) Graphics Compiler](https://github.com/intel/intel-graphics-compiler) (e.g. the `libigc2` package)

## Build and Run

**Note**: This sample does not support IGC's latest binary format (ZEBinary).
To use this sample, run `export IGC_EnableZEBinary=0` before running the sample.

### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/ze_debug_info
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./ze_debug_info <target_application>
```
One may use [ze_gemm](../ze_gemm) as target application:
```sh
./ze_debug_info ../../ze_gemm/build/ze_gemm
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\ze_debug_info
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<level_zero_loader>\lib;<iga_lib_path> -DCMAKE_INCLUDE_PATH=<level_zero_loader>\include ..
nmake
```
Use this command line to run the tool:
```sh
ze_debug_info.exe <target_application>
```
One may use [ze_gemm](../ze_gemm) as target application:
```sh
ze_debug_info.exe ..\..\ze_gemm\build\ze_gemm.exe
```
**Note**: to build this sample one may need to generate *.lib file from IGA *.dll (see [here](https://stackoverflow.com/questions/9946322/how-to-generate-an-import-library-lib-file-from-a-dll) for details) and provide the path to this *.lib to cmake with `-DCMAKE_LIBRARY_PATH`.

Also one may need to add an actual path to IGA *.dll into PATH before sample run, e.g.:
```
set PATH=%PATH%;<iga_dll_path>
ze_debug_info.exe ..\..\ze_gemm\build\ze_gemm.exe
```
