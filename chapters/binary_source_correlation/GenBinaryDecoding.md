# GEN Binary Decoding
## Overview
This chapter describes [Intel(R) Processor Graphics Assembler (IGA)](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary) library - a part of [Intel(R) Processor Graphics Compiler](https://github.com/intel/intel-graphics-compiler). The IGA is designed to decode and disassemble GEN binaries, including getting instruction properties, like opcode, instruction size, arguments, execution size, etc.

**Supported OS**:
- Linux
- Windows (not validated)

**Supported HW**:
- Tested on Intel(R) Data Center GPU Max 1550 (Xe-HPC, i915 driver) and Intel(R) Arc(TM) B570 Graphics (Xe2, xe driver); other GPUs, including older generations, are not tested

**Needed Headers**:
- Intel(R) Processor Graphics Assembler (IGA) library [headers](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary/api)

**Needed Libraries**:
- Intel(R) Processor Graphics Assembler (IGA) [library](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary), can be installed as part of [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)

## How To Use
An example below demonstrates how to split raw GEN binary into separate instructions and disassemble them using `KernelView` interface of IGA:
```cpp
#include <kv.hpp>

// arch is the IGA platform of the device that the binary was built for,
// e.g. IGA_XE_HPC (Intel(R) Data Center GPU Max) or XE_VER(2, 0) (IGA_XE2,
// Intel(R) Arc(TM) B-Series). GenBinaryDecoder::GfxCoreToIgaGen() in
// utils/gen_binary_decoder.h maps the device GFX core family to it.
std::vector< std::pair<uint64_t, std::string> > Disassemble(
    const std::vector<uint8_t>& binary, iga_gen_t arch) {
  std::vector< std::pair<uint64_t, std::string> > instruction_list;

  KernelView kv(arch, binary.data(), binary.size());
  assert(kv.decodeSucceeded());

  char text[MAX_STR_SIZE] = { 0 };
  uint64_t offset = 0, size = 0;
  while (true) {
    size = kv.getInstSize(offset);
    if (size == 0) {
      break;
    }

    size_t length = kv.getInstSyntax(offset, text, MAX_STR_SIZE);
    assert(length > 0);
    instruction_list.push_back(std::make_pair(offset, text));

    offset += size;
  }

  return instruction_list;
}
```

## Usage Details
- refer to the paper [Introduction to GEN Assembly](https://www.intel.com/content/www/us/en/developer/articles/technical/introduction-to-gen-assembly.html) to learn more on Intel(R) Processor Graphics instruction set
- look into Intel(R) Processor Graphics Assembler (IGA) library [headers](https://github.com/intel/intel-graphics-compiler/tree/master/visa/iga/IGALibrary/api) to learn more on GEN binary decoding/disassembling interfaces

## Samples
- [OpenCL(TM) Debug Info](../../samples/cl_debug_info)
- [Level Zero Debug Info](../../samples/ze_debug_info)

## Tools
- [GPU Instruction Count](../../tools/instcount)
