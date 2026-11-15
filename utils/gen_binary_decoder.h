//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#ifndef PTI_UTILS_GEN_BINARY_DECODER_H_
#define PTI_UTILS_GEN_BINARY_DECODER_H_

#include <iga/kv.hpp>
#include <string>
#include <vector>

#include "utils.h"

struct Instruction {
  uint64_t offset;
  std::string text;
};

typedef uint32_t gfx_core_family_t;

class GenBinaryDecoder {
 public:
  GenBinaryDecoder(const std::vector<uint8_t>& binary, iga_gen_t arch)
      : kernel_view_(arch, binary.data(), binary.size()) {}
  GenBinaryDecoder(const uint8_t* data, uint32_t size, iga_gen_t arch)
      : kernel_view_(arch, data, size) {}

  bool IsValid() const { return kernel_view_.decodeSucceeded(); }

  std::vector<Instruction> Disassemble() {
    if (!IsValid()) {
      return std::vector<Instruction>();
    }

    std::vector<Instruction> instruction_list;

    char text[MAX_STR_SIZE] = {0};
    uint64_t offset = 0, size = 0;
    while (true) {
      size = kernel_view_.getInstSize(offset);
      if (size == 0) {
        break;
      }

      size_t length = kernel_view_.getInstSyntax(offset, text, MAX_STR_SIZE);
      PTI_ASSERT(length > 0);
      instruction_list.push_back({offset, text});

      offset += size;
    }

    return instruction_list;
  }

  /**
   * @brief Convert GFX core family enum to iga_gen_t enum.
   */
  static iga_gen_t GfxCoreToIgaGen(gfx_core_family_t type) {
    // There is not official conversion function from GFXCORE_FAMILY to IGA_GEN. Next information
    // was used for this conversation: GfxCore to visa platform: IGC::GetVISAPlatform @
    // https://github.com/intel/intel-graphics-compiler/blob/master/IGC/Compiler/CISACodeGen/CISABuilder.cpp
    // visa platform to iga_gen_t: BinaryEncodingIGA::getIGAInternalPlatform @
    // https://github.com/intel/intel-graphics-compiler/blob/master/visa/BinaryEncodingIGA.cpp
    //
    // GFXCORE_FAMILY are described in "inc/common/igfxfmid.h"

    switch (type) {
      case 12:  // GFXCORE_FAMILY::IGFX_GEN9_CORE:
        return IGA_GEN9;
      case 13:  // GFXCORE_FAMILY::IGFX_GEN10_CORE:
      case 14:  // GFXCORE_FAMILY::IGFX_GEN10LP_CORE:
        return IGA_GEN10;
      case 15:  // GFXCORE_FAMILY::IGFX_GEN11_CORE:
      case 16:  // GFXCORE_FAMILY::IGFX_GEN11LP_CORE:
        return IGA_GEN11;
      case 17:  // GFXCORE_FAMILY::IGFX_GEN12_CORE:
      case 18:  // GFXCORE_FAMILY::IGFX_GEN12LP_CORE:
        return IGA_GEN12p1;
      case 0x0c05:  // GFXCORE_FAMILY::IGFX_XE_HP_CORE:
        return IGA_XE_HP;
      case 0x0c07:  // GFXCORE_FAMILY::IGFX_XE_HPG_CORE:
        return IGA_XE_HPG;
      case 0x0c08:  // GFXCORE_FAMILY::IGFX_XE_HPC_CORE:
        return IGA_XE_HPC;
      // IGA_XE2 / IGA_XE3 are spelled as XE_VER() values: older system iga.h
      // headers don't have these enumerators but do have the macro.
      case 0x0c09:  // GFXCORE_FAMILY::IGFX_XE2_LPG_CORE (= IGFX_XE2_HPG_CORE): LNL, BMG
        return static_cast<iga_gen_t>(XE_VER(2, 0));  // IGA_XE2
      case 0x1e00:  // GFXCORE_FAMILY::IGFX_XE3_CORE: PTL
        return static_cast<iga_gen_t>(XE_VER(3, 0));  // IGA_XE3
      // IGFX_XE3P_CORE (0x2300) is left out: IGA only has the XPC variant of it
      // (IGA_XE3P_XPC), and other Xe3P parts may need another IGA platform.
      default:
        return IGA_GEN_INVALID;
    }
  }

 private:
  KernelView kernel_view_;
};

#endif  // PTI_UTILS_GEN_BINARY_DECODER_H_
