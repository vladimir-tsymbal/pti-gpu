# OpenCL(TM) Debug Info
## Overview
This sample is a simple LD_PRELOAD based tool that allows to collect OpenCL kernel sources, binaries and symbols to correlate them. This tool works for Intel(R) Processor Graphics only.

As a result, listing like the following will be printed for each kernel (once per kernel creation).
```
===== Kernel: GEMM =====
=== File: Unknown ===
                [0x00430]         illegal
=== File: Kernel Source ===
                [0x001F0] (W)     mul (16|M0)              acc0.0<1>:d   r10.0<1;1,0>:d    r5.4<0;1,0>:uw   {Compacted,I@2}
                [0x001F8]         macl (16|M0)             r12.0<1>:d    r10.0<1;1,0>:d    r5.2<0;1,0>:d    {Compacted}
                [0x00200] (W)     mul (16|M16)             acc0.0<1>:d   r11.0<1;1,0>:d    r5.4<0;1,0>:uw   {Compacted}
[    1] __kernel void GEMM(__global float* a, __global float* b,
                [0x00000] (W)     mov (16|M0)              r127.0<1>:ud  0x0:ud
                [0x00010] (W)     and (1|M0)               r127.2<1>:ud  r0.0<0;1,0>:ud    0xFFFFFFC0:ud
                [0x00020] (W)     and (1|M0)               r127.0<1>:uw  r0.4<0;1,0>:uw    0xFF:uw
                [0x00030] (W)     add (1|M0)               r127.2<1>:ud  r127.2<0;1,0>:ud  0x60:ud              {I@2}
                [0x00040] (W)     add (1|M0)               r127.2<1>:ud  r127.2<0;1,0>:ud  0x0:ud              {I@1}
                [0x00050] (W)     mad (1|M0)               r127.0<1>:ud  r127.2<0;0>:ud    r127.0<0;0>:uw    0xC0:uw              {I@1}
                [0x00060] (W)     send.ugm (1|M0)          r1       r127    null:0  0xFF000000            0x6228E500           {A@1,$0} // wr:1+0, rd:2; load.ugm.d32x32t.a32.ca.ca.bti[255]
                [0x00070] (W)     add (1|M0)               r126.0<1>:ud  r127.0<0;1,0>:ud  0x80:uw
                [0x00080] (W)     send.ugm (1|M0)          r3       r126    null:0  0xFF000000            0x6218D500           {A@1,$1} // wr:1+0, rd:1; load.ugm.d32x16t.a32.ca.ca.bti[255]
                [0x00090] (W)     mov (1|M0)               null<1>:ud    0x30433:ud                              {$1.src}
                [0x000A0] (W)     mov (1|M0)               null<1>:ud    r127.0<0;1,0>:ud
                [0x000B0] (W)     mov (1|M0)               null<1>:ud    r126.0<0;1,0>:ud
                [0x000C0]         sync.nop                             null                             {I@1}
                [0x000D0]         sync.allrd                           ($0)
                [0x000E0]         nop
                [0x000F0]         nop
                [0x00100] (W)     and (1|M0)               r127.0<1>:ud  r0.0<0;1,0>:ud    0xFFFFFFC0:ud
                [0x00110] (W)     add (1|M0)               r127.0<1>:ud  r127.0<0;1,0>:ud  0x0:ud              {I@1}
                [0x00120] (W)     send.ugm (1|M0)          r4       r127    null:0  0xFF000000            0x6218D500           {I@1,$2} // wr:1+0, rd:1; load.ugm.d32x16t.a32.ca.ca.bti[255]
                [0x00130] (W)     add (1|M0)               r126.0<1>:ud  r127.0<0;1,0>:ud  0x40:uw
                [0x00140] (W)     send.ugm (1|M0)          r5       r126    null:0  0xFF000000            0x6218C500           {I@1,$3} // wr:1+0, rd:1; load.ugm.d32x8t.a32.ca.ca.bti[255]
                [0x00150] (W)     mov (1|M0)               null<1>:ud    0x30433:ud                              {$3.src}
                [0x00160] (W)     mov (1|M0)               null<1>:ud    r127.0<0;1,0>:ud                 {Compacted}
                [0x00168] (W)     mov (1|M0)               null<1>:ud    r126.0<0;1,0>:ud                 {Compacted}
                [0x00170]         sync.nop                             null                             {Compacted,I@1}
                [0x00178]         sync.allrd                           ($2)                 {Compacted}
                [0x00180] (W)     or (1|M0)                cr0.0<1>:ud   cr0.0<0;1,0>:ud   0x4C0:uw              {Compacted,A@1}
[    2]                    __global float* c, unsigned size) {
[    3]   int j = get_global_id(0);
                [0x00188] (W)     mul (1|M0)               acc0.0<1>:d   r4.8<0;1,0>:d     r0.2<0;1,0>:uw   {Compacted,A@1,$2.dst}
                [0x00190] (W)     macl (1|M0)              r3.0<1>:d     r4.8<0;1,0>:d     r0.1<0;1,0>:d    {Compacted,$1.dst}
                [0x001E0]         add (32|M0)              r7.0<1>:d     r3.0<0;1,0>:d     r1.0<1;1,0>:uw
...
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
- Intel(R) Processor Graphics Assembler (IGA) library `libiga64.so`, part of the [Intel(R) Graphics Compiler](https://github.com/intel/intel-graphics-compiler) (e.g. the `libigc2` package)

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/samples/cl_debug_info
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./cl_debug_info <target_application>
```
One may use [cl_gemm](../cl_gemm) as target application:
```sh
./cl_debug_info ../../cl_gemm/build/cl_gemm gpu
```
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\samples\cl_debug_info
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path>;<iga_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
cl_debug_info.exe <target_application>
```
One may use [cl_gemm](../cl_gemm) as target application:
```sh
cl_debug_info.exe ..\..\cl_gemm\build\cl_gemm.exe
```
**Note**: to build this sample one may need to generate *.lib file from IGA *.dll (see [here](https://stackoverflow.com/questions/9946322/how-to-generate-an-import-library-lib-file-from-a-dll) for details) and provide the path to this *.lib to cmake with `-DCMAKE_LIBRARY_PATH`.

Also one may need to add an actual path to IGA *.dll into PATH before sample run, e.g.:
```
set PATH=%PATH%;<iga_dll_path>
cl_debug_info.exe ..\..\cl_gemm\build\cl_gemm.exe
```
