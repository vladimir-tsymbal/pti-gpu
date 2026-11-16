# Binary/Source Correlation for oneAPI Level Zero (Level Zero)
## Overview
This chapter describes the interfaces and solutions one can use to deal with oneAPI Level Zero kernel sources, binaries and symbols.

**Supported Runtimes**:
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

**Supported OS**:
- Linux
- Windows (not validated)

**Supported HW**:
- Tested on Intel(R) Data Center GPU Max 1550 (Xe-HPC, i915 driver) and Intel(R) Arc(TM) B570 Graphics (Xe2, xe driver); other GPUs, including older generations, are not tested

**Needed Headers**:
- [zet_api.h](https://github.com/oneapi-src/level-zero/blob/master/include/zet_api.h)
- [patch_list.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/patch_list.h)
- [program_debug_data.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/program_debug_data.h)
- Intel(R) Processor Graphics Assembler (IGA) [headers](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary/api)

**Needed Libraries**:
- oneAPI Level Zero [libraries](https://github.com/intel/compute-runtime)
- Intel(R) Processor Graphics Assembler (IGA) [library](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary), can be installed as part of [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

## How To Use
### Binary Capture
Kernel binary grabbing is the feature of oneAPI Level Zero core API that is implemented by `zeModuleGetNativeBinary` function:
```cpp
ze_result_t status = ZE_RESULT_SUCCESS;
size_t binary_size = 0;
status = zeModuleGetNativeBinary(module, &binary_size, nullptr);
assert(status == ZE_RESULT_SUCCESS);

std::vector<uint8_t> binary(binary_size);
status = zeModuleGetNativeBinary(module, &binary_size, binary.data());
assert(status == ZE_RESULT_SUCCESS);
```

### Binary Parsing
**Current drivers:** with compute-runtime 25.18 (Intel(R) Data Center GPU Max 1550), 26.05 and 26.40 (Intel(R) Arc(TM) B570 Graphics), the module binary returned by `zeModuleGetNativeBinary` is a *zebin*: a standard ELF64 file (`e_machine` 205, Intel(R) GT) with the code of each kernel in its own `.text.<kernel name>` section, plus `.symtab` and the DWARF `.debug_*` sections (format: [zebin_elf.h](https://github.com/intel/compute-runtime/blob/master/shared/source/device_binary_format/zebin/zebin_elf.h)). The samples parse it with the [GPU ELF parser](../../utils/gpu_elf_parser) of this repository. The legacy format described below (`"Intel(R) OpenCL Device Binary"` section in the [patch_list.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/patch_list.h) layout) is not produced by these drivers and is not tested.

Module binary obtained by `zeModuleGetNativeBinary` is stored as `"Intel(R) OpenCL Device Binary"` section of ELF file in a special internal format described in [patch_list.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/patch_list.h).

To retrieve raw GEN binary for a kernel with a specific name, one should perform the next two steps.

The first step is to parse binary data in standard ELF64 format to get the content of `"Intel(R) OpenCL Device Binary"` section (let's call it `igc_binary`).

The second step is to parse this `igc_binary` that is stored in an internal [Intel(R) Processor Graphics Compiler](https://github.com/intel/intel-graphics-compiler) format described in [patch_list.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/patch_list.h):
```cpp
#include <patch_list.h>

const SProgramBinaryHeader* header =
  reinterpret_cast<const SProgramBinaryHeader*>(igc_binary.data());
assert(header->Magic == MAGIC_CL);

const uint8_t* ptr = reinterpret_cast<const uint8_t*>(header) +
  sizeof(SProgramBinaryHeader) + header->PatchListSize;
for (uint32_t i = 0; i < header->NumberOfKernels; ++i) {
  const SKernelBinaryHeaderCommon* kernel_header =
    reinterpret_cast<const SKernelBinaryHeaderCommon*>(ptr);

  ptr += sizeof(SKernelBinaryHeaderCommon);
  const char* kernel_name = reinterpret_cast<const char*>(ptr);

  ptr += kernel_header->KernelNameSize;
  if (strcmp(kernel_name, "SomeKernel") == 0) {
    std::vector<uint8_t> raw_binary(kernel_header->KernelHeapSize);
    memcpy(raw_binary.data(), ptr,
           kernel_header->KernelHeapSize * sizeof(uint8_t));
  }

  ptr += kernel_header->PatchListSize +
    kernel_header->KernelHeapSize +
    kernel_header->GeneralStateHeapSize + kernel_header->DynamicStateHeapSize +
    kernel_header->SurfaceStateHeapSize;
}
```

### Binary Decoding/Disassembling
To decode and/or disassemble GEN binaries one should use [Intel(R) Processor Graphics Assembler (IGA)](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary) library described [here](../../chapters/binary_source_correlation/GenBinaryDecoding.md).

### Symbols Capture
Debug symbols grabbing for a module is a feature of Level Zero Tools API that is implemented by function `zetModuleGetDebugInfo`:
```cpp
size_t debug_info_size = 0;
status = zetModuleGetDebugInfo(
    module, ZET_MODULE_DEBUG_INFO_FORMAT_ELF_DWARF,
    &debug_info_size, nullptr);
assert(status == ZE_RESULT_SUCCESS);
assert(debug_info_size > 0);

std::vector<uint8_t> debug_info(debug_info_size);
status = zetModuleGetDebugInfo(
    module, ZET_MODULE_DEBUG_INFO_FORMAT_ELF_DWARF,
    &debug_info_size, debug_info.data());
assert(status == ZE_RESULT_SUCCESS);
```

### Symbols Parsing
**Current drivers:** with compute-runtime 25.18, 26.05 and 26.40 the symbols returned by `zetModuleGetDebugInfo` are a zebin ELF64 file as well, with the kernel code and the DWARF `.debug_info`, `.debug_line` and related sections. The [Debug Info](../../samples/ze_debug_info) sample reads the kernel code and the source line mapping from it with the [GPU ELF parser](../../utils/gpu_elf_parser). The IGC container format described [here](GenSymbolsDecoding.md) is not produced by these drivers and is not tested.

To decode debug symbols for GPU modules one should refer to [Intel(R) Processor Graphics Compiler (IGC)](https://github.com/intel/intel-graphics-compiler) internal formats described [here](GenSymbolsDecoding.md).

## Usage Details
- refer to oneAPI Level Zero [documentation](https://oneapi-src.github.io/level-zero-spec/level-zero/latest/index.html) to learn more
- look into GEN binary decoding [chapter](GenBinaryDecoding.md) to learn more on GEN binary decoding/disassembling interfaces
- look into GEN symbols decoding [chapter](GenSymbolsDecoding.md) to learn more on symbols format
- refer to the IGC [patch_list.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/patch_list.h) header to learn more on module binary layout
- refer to the IGC [program_debug_data.h](https://github.com/intel/intel-graphics-compiler/blob/master/IGC/AdaptorOCL/ocl_igc_shared/executable_format/program_debug_data.h) header to learn more on debug symbols layout

## Samples
- [Level Zero Debug Info](../../samples/ze_debug_info)