# GPU Instruction Count

## Overview

This tool allows to collect dynamic instruction execution count and SIMD active count for every GPU kernel instruction.

As a result, assembly listing annotated with dynamic instruction count and SIMD active lanes for each kernel will be printed.

```console

[INFO] : [ Instruction count | SIMD active lanes count ] total for all invocations
=== GEMM (runs 1 times)
///  Basic block #0
[    32768|      32768] 0x000180 : (W)     or (1|M0)                cr0.0<1>:ud   cr0.0<0;1,0>:ud   0x4C0:uw              {Compacted,A@1}
[    32768|      32768] 0x000188 : (W)     mul (1|M0)               acc0.0<1>:d   r4.8<0;1,0>:d     r0.2<0;1,0>:uw   {Compacted,A@1,$2.dst}
[    32768|      32768] 0x000190 : (W)     macl (1|M0)              r3.0<1>:d     r4.8<0;1,0>:d     r0.1<0;1,0>:d    {Compacted,$1.dst}
[    32768|      32768] 0x000198 : (W)     mul (1|M0)               acc0.0<1>:d   r4.9<0;1,0>:d     r0.12<0;1,0>:uw
[    32768|      32768] 0x0001a8 : (W)     macl (1|M0)              r9.0<1>:d     r4.9<0;1,0>:d     r0.6<0;1,0>:d
[    32768|    1048576] 0x0001b8 : (W)     cmp (32|M0)   (eq)f0.0   null<1>:d     r5.2<0;1,0>:d     0:w               {$3.dst}
[    32768|      32752] 0x0001c8 :         sync.nop                             null                             {Compacted,I@2}
[    32768|    1048428] 0x0001d0 :         add3 (32|M0)             r10.0<1>:d    r9.0<0;0>:d       r2.0<1;0>:uw      r4.1<0>:d        {$0.dst}
[    32768|    1048428] 0x0001e0 :         add (32|M0)              r7.0<1>:d     r3.0<0;1,0>:d     r1.0<1;1,0>:uw
[    32768|     524288] 0x0001f0 : (W)     mul (16|M0)              acc0.0<1>:d   r10.0<1;1,0>:d    r5.4<0;1,0>:uw   {Compacted,I@2}
[    32768|     523967] 0x0001f8 :         macl (16|M0)             r12.0<1>:d    r10.0<1;1,0>:d    r5.2<0;1,0>:d    {Compacted}
[    32768|     524288] 0x000200 : (W)     mul (16|M16)             acc0.0<1>:d   r11.0<1;1,0>:d    r5.4<0;1,0>:uw   {Compacted}
[    32768|     523997] 0x000208 :         macl (16|M16)            r13.0<1>:d    r11.0<1;1,0>:d    r5.2<0;1,0>:d    {Compacted}
[    32768|      32768] 0x000210 : (W&~f0.0) jmpi                               40
///  Basic block #1
[        0|          0] 0x000220 :         mov (32|M0)              r14.0<1>:ud   0x0:ud                              {Compacted}
[        0|          0] 0x000228 : (W)     jmpi                                 280
///  Basic block #2
[    32768|     524213] 0x000238 :         mov (16|M0)              r38.0<2>:ud   r12.0<1;1,0>:ud                  {Compacted,I@6}
[    32768|     524112] 0x000240 :         mov (16|M16)             r40.0<2>:ud   r13.0<1;1,0>:ud                  {Compacted,I@5}
[    32768|    1048276] 0x000248 :         mov (32|M0)              r14.0<1>:ud   0x0:ud                              {Compacted}
[    32768|     524213] 0x000250 :         shl (16|M0)              r50.0<1>:q    r38.0<2;1,0>:ud   2:w               {Compacted,I@3}
[    32768|     524112] 0x000258 :         shl (16|M16)             r52.0<1>:q    r40.0<2;1,0>:ud   2:w               {Compacted,I@3}
[    32768|     524213] 0x000260 :         add (16|M0)              r16.0<1>:q    r50.0<1;1,0>:q    r4.6<0;1,0>:q    {Compacted,I@2}
[    32768|     524112] 0x000268 :         add (16|M16)             r18.0<1>:q    r52.0<1;1,0>:q    r4.6<0;1,0>:q    {Compacted,I@2}
[    32768|      32768] 0x000270 : (W)     mov (1|M0)               r3.1<1>:d     0:w                               {Compacted}
///  Basic block #3
[ 33554432|   33554432] 0x000278 : (W)     mul (1|M0)               acc0.0<1>:d   r3.1<0;1,0>:d     r5.4<0;1,0>:uw   {Compacted,I@1}
[ 33554432|   33554432] 0x000280 : (W)     macl (1|M0)              r23.0<1>:d    r3.1<0;1,0>:d     r5.2<0;1,0>:d    {Compacted}
[ 33554432| 1073371759] 0x000288 :         send.ugm (32|M0)         r21      r16     null:0  0x0            0x08200580           {$4} // wr:4+0, rd:2; load.ugm.d32.a64
[ 33554432| 1073518075] 0x000298 :         add3 (32|M0)             r24.0<1>:d    r23.0<0;0>:d      r7.0<1;0>:d       r4.0<0>:d        {Compacted,I@1}
[ 33554432|  536665541] 0x0002a0 :         mov (16|M0)              r42.0<2>:ud   r24.0<1;1,0>:ud                  {Compacted,I@1}
[ 33554432|  536686289] 0x0002a8 :         mov (16|M16)             r44.0<2>:ud   r25.0<1;1,0>:ud                  {Compacted}
[ 33554432|  536665541] 0x0002b0 :         shl (16|M0)              r54.0<1>:q    r42.0<2;1,0>:ud   2:w               {Compacted,I@2}
[ 33554432|  536686289] 0x0002b8 :         shl (16|M16)             r56.0<1>:q    r44.0<2;1,0>:ud   2:w               {Compacted,I@2}
[ 33554432|  536665541] 0x0002c0 :         add (16|M0)              r26.0<1>:q    r54.0<1;1,0>:q    r4.7<0;1,0>:q    {Compacted,I@2}
[ 33554432|  536686289] 0x0002c8 :         add (16|M16)             r28.0<1>:q    r56.0<1;1,0>:q    r4.7<0;1,0>:q    {Compacted,I@2}
[ 33554432| 1073371759] 0x0002d0 :         send.ugm (32|M0)         r30      r26     null:0  0x0            0x08200580           {A@1,$5} // wr:4+0, rd:2; load.ugm.d32.a64
[ 33554432|   33554432] 0x0002e0 : (W)     add (1|M0)               r3.1<1>:d     r3.1<0;1,0>:d     1:w
[ 33554432| 1073741824] 0x0002f0 : (W)     cmp (32|M0)   (lt)f1.0   null<1>:ud    r3.1<0;1,0>:ud    r5.2<0;1,0>:ud   {I@1}
[ 33554432|   33549315] 0x000300 :         sync.nop                             null                             {Compacted,$5.dst}
[ 33554432| 1073266701] 0x000308 :         mad (32|M0)              r14.0<1>:f    r14.0<1;0>:f      r21.0<1;0>:f      r30.0<1>:f       {Compacted,$4.dst}
[ 33554432|      32768] 0x000310 : (W&~f1.0) jmpi                               48
///  Basic block #4
[ 33521664|  536142985] 0x000320 :         add (16|M0)              r16.0<1>:q    r16.0<1;1,0>:q    4:w               {Compacted}
[ 33521664|  536162036] 0x000328 :         add (16|M16)             r18.0<1>:q    r18.0<1;1,0>:q    4:w               {Compacted}
[ 33521664|   33521664] 0x000330 : (W)     jmpi                                 -184
///  Basic block #5
[    32768|    1048213] 0x000340 :         add3 (32|M0)             r32.0<1>:d    r12.0<1;0>:d      r7.0<1;0>:d       r4.0<0>:d        {Compacted}
[    32768|     524142] 0x000348 :         mov (16|M0)              r46.0<2>:ud   r32.0<1;1,0>:ud                  {Compacted,I@1}
[    32768|     524272] 0x000350 :         mov (16|M16)             r48.0<2>:ud   r33.0<1;1,0>:ud                  {Compacted}
[    32768|     524142] 0x000358 :         shl (16|M0)              r58.0<1>:q    r46.0<2;1,0>:ud   2:w               {Compacted,I@2}
[    32768|     524272] 0x000360 :         shl (16|M16)             r60.0<1>:q    r48.0<2;1,0>:ud   2:w               {Compacted,I@2}
[    32768|     524142] 0x000368 :         add (16|M0)              r34.0<1>:q    r58.0<1;1,0>:q    r5.0<0;1,0>:q    {Compacted,I@2}
[    32768|     524272] 0x000370 :         add (16|M16)             r36.0<1>:q    r60.0<1;1,0>:q    r5.0<0;1,0>:q    {Compacted,I@2}
[    32768|    1048545] 0x000378 :         send.ugm (32|M0)         null     r34     r14:2   0x0            0x08000584           {A@1,$6} // wr:4+2, rd:0; store.ugm.d32.a64
[    32768|     524288] 0x000388 : (W)     mov (16|M0)              r127.0<1>:ud  r0.0<1;1,0>:ud                   {Compacted}
///  Basic block #6
[    32768|      32768] 0x000390 : (W)     send.gtwy (1|M0)         null     r127    null:0  0x0            0x02000010           {EOT,I@1,$7} // wr:1+0, rd:0; end of thread
///  Basic block #7
[        0|          0] 0x0003a0 :         nop
```

The following capabilities are available:
```console
--disable-simd                 Disable SIMD active lanes collection
--json-output                  Print results in JSON format
```

## Supported OS

- Ubuntu 24.04 with the i915 kernel driver on Intel(R) Data Center GPU Max 1550, and Ubuntu 26.04 with the xe kernel driver on Intel(R) Arc(TM) B570 Graphics (last check: GT Pin 5.0.0, compute-runtime 25.18 and 26.05; versions in [SOFTWARE](../../SOFTWARE))
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
cd <pti>/tools/instcount
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release [-DGTPIN_PATH=<gtpin>] ..
make
```

Use this command line to run the tool:

```sh
./instcount <target_application>
```

One may use [cl_gemm](../../samples/cl_gemm), [ze_gemm](../../samples/ze_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application:

```sh
./instcount ../../../samples/cl_gemm/build/cl_gemm
./instcount ../../../samples/ze_gemm/build/ze_gemm
./instcount ../../../samples/dpc_gemm/build/dpc_gemm
```

### Windows

Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:

```sh
cd <pti>\tools\instcount
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
nmake
```

Use this command line to run the tool:

```sh
set PATH=%PATH%;<gtpin>\Profilers\Lib\intel64
instcount.exe <target_application>
```

CMake unpacks GTPin into "_deps\gtpin_package-src\Profilers":

```sh
set PATH=%PATH%;_deps\gtpin_package-src\Profilers\Lib\intel64
instcount.exe <target_application>
```

One may use [cl_gemm](../../samples/cl_gemm), [ze_gemm](../../samples/ze_gemm) or [dpc_gemm](../../samples/dpc_gemm) as target application:

```sh
set PATH=%PATH%;<gtpin>\Profilers\Lib\intel64
instcount.exe ..\..\cl_gemm\build\cl_gemm.exe
instcount.exe ..\..\ze_gemm\build\ze_gemm.exe
instcount.exe ..\..\dpc_gemm\build\dpc_gemm.exe
```

**Note**: to build this sample one may need to generate \*.lib file from IGA \*.dll (see [here](https://stackoverflow.com/questions/9946322/how-to-generate-an-import-library-lib-file-from-a-dll) for details) and provide the path to this \*.lib to cmake with `-DCMAKE_LIBRARY_PATH`.

Also one may need to add an actual path to IGA *.dll into PATH before sample run, e.g.:

```sh
set PATH=%PATH%;<gtpin>\Profilers\Lib\intel64
set PATH=%PATH%;<iga_dll_path>
instcount.exe ..\..\cl_gemm\build\cl_gemm.exe
```

The options can be one or more of the following:
```console
--disable-simd                 Disable SIMD active lanes collection
--json-output                  Print results in JSON format
--version                      Print version
```
