//===-- SIProgramInfo.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
///
/// The SIProgramInfo tracks resource usage and hardware flags for kernels and
/// entry functions.
//
//===----------------------------------------------------------------------===//
//

#include "SIProgramInfo.h"
#include "GCNSubtarget.h"
#include "SIDefines.h"
#include "Utils/AMDGPUBaseInfo.h"
#include "llvm/MC/MCExpr.h"

using namespace llvm;

void SIProgramInfo::reset(const MachineFunction &MF) {
  MCContext &Ctx = MF.getContext();

  const MCExpr *ZeroExpr = MCConstantExpr::create(0, Ctx);

  VGPRBlocks = ZeroExpr;
  SGPRBlocks = ZeroExpr;
  Priority = 0;
  FloatMode = 0;
  Priv = 0;
  DX10Clamp = 0;
  DebugMode = 0;
  IEEEMode = 0;
  WgpMode = 0;
  MemOrdered = 0;
  RrWgMode = 0;
  ScratchSize = ZeroExpr;

  LDSBlocks = 0;
  ScratchBlocks = ZeroExpr;

  ScratchEnable = ZeroExpr;
  UserSGPR = 0;
  TrapHandlerEnable = 0;
  TGIdXEnable = 0;
  TGIdYEnable = 0;
  TGIdZEnable = 0;
  TGSizeEnable = 0;
  TIdIGCompCount = 0;
  EXCPEnMSB = 0;
  LdsSize = 0;
  EXCPEnable = 0;

  ComputePGMRSrc3GFX90A = 0;

  NumVGPR = ZeroExpr;
  NumArchVGPR = ZeroExpr;
  NumAccVGPR = ZeroExpr;
  AccumOffset = ZeroExpr;
  TgSplit = 0;
  NumSGPR = ZeroExpr;
  SGPRSpill = 0;
  VGPRSpill = 0;
  LDSSize = 0;
  FlatUsed = ZeroExpr;

  NumSGPRsForWavesPerEU = ZeroExpr;
  NumVGPRsForWavesPerEU = ZeroExpr;
  Occupancy = ZeroExpr;
  DynamicCallStack = ZeroExpr;
  VCCUsed = ZeroExpr;
}

uint64_t SIProgramInfo::getComputePGMRSrc1(const GCNSubtarget &ST) const {
  int64_t VBlocks, SBlocks;
  VGPRBlocks->evaluateAsAbsolute(VBlocks);
  SGPRBlocks->evaluateAsAbsolute(SBlocks);

  uint64_t Reg = S_00B848_VGPRS(static_cast<uint64_t>(VBlocks)) |
                 S_00B848_SGPRS(static_cast<uint64_t>(SBlocks)) |
                 S_00B848_PRIORITY(Priority) | S_00B848_FLOAT_MODE(FloatMode) |
                 S_00B848_PRIV(Priv) | S_00B848_DEBUG_MODE(DebugMode) |
                 S_00B848_WGP_MODE(WgpMode) | S_00B848_MEM_ORDERED(MemOrdered);

  if (ST.hasDX10ClampMode())
    Reg |= S_00B848_DX10_CLAMP(DX10Clamp);

  if (ST.hasIEEEMode())
    Reg |= S_00B848_IEEE_MODE(IEEEMode);

  if (ST.hasRrWGMode())
    Reg |= S_00B848_RR_WG_MODE(RrWgMode);

  return Reg;
}

uint64_t SIProgramInfo::getPGMRSrc1(CallingConv::ID CC,
                                    const GCNSubtarget &ST) const {
  if (AMDGPU::isCompute(CC)) {
    return getComputePGMRSrc1(ST);
  }
  int64_t VBlocks, SBlocks;
  VGPRBlocks->evaluateAsAbsolute(VBlocks);
  SGPRBlocks->evaluateAsAbsolute(SBlocks);

  uint64_t Reg = S_00B848_VGPRS(static_cast<uint64_t>(VBlocks)) |
                 S_00B848_SGPRS(static_cast<uint64_t>(SBlocks)) |
                 S_00B848_PRIORITY(Priority) | S_00B848_FLOAT_MODE(FloatMode) |
                 S_00B848_PRIV(Priv) | S_00B848_DEBUG_MODE(DebugMode);

  if (ST.hasDX10ClampMode())
    Reg |= S_00B848_DX10_CLAMP(DX10Clamp);

  if (ST.hasIEEEMode())
    Reg |= S_00B848_IEEE_MODE(IEEEMode);

  if (ST.hasRrWGMode())
    Reg |= S_00B848_RR_WG_MODE(RrWgMode);

  switch (CC) {
  case CallingConv::AMDGPU_PS:
    Reg |= S_00B028_MEM_ORDERED(MemOrdered);
    break;
  case CallingConv::AMDGPU_VS:
    Reg |= S_00B128_MEM_ORDERED(MemOrdered);
    break;
  case CallingConv::AMDGPU_GS:
    Reg |= S_00B228_WGP_MODE(WgpMode) | S_00B228_MEM_ORDERED(MemOrdered);
    break;
  case CallingConv::AMDGPU_HS:
    Reg |= S_00B428_WGP_MODE(WgpMode) | S_00B428_MEM_ORDERED(MemOrdered);
    break;
  default:
    break;
  }
  return Reg;
}

uint64_t SIProgramInfo::getComputePGMRSrc2() const {
  int64_t ScratchEn;
  ScratchEnable->evaluateAsAbsolute(ScratchEn);
  uint64_t Reg =
      S_00B84C_SCRATCH_EN(ScratchEn) | S_00B84C_USER_SGPR(UserSGPR) |
      S_00B84C_TRAP_HANDLER(TrapHandlerEnable) |
      S_00B84C_TGID_X_EN(TGIdXEnable) | S_00B84C_TGID_Y_EN(TGIdYEnable) |
      S_00B84C_TGID_Z_EN(TGIdZEnable) | S_00B84C_TG_SIZE_EN(TGSizeEnable) |
      S_00B84C_TIDIG_COMP_CNT(TIdIGCompCount) |
      S_00B84C_EXCP_EN_MSB(EXCPEnMSB) | S_00B84C_LDS_SIZE(LdsSize) |
      S_00B84C_EXCP_EN(EXCPEnable);

  return Reg;
}

uint64_t SIProgramInfo::getPGMRSrc2(CallingConv::ID CC) const {
  if (AMDGPU::isCompute(CC))
    return getComputePGMRSrc2();

  return 0;
}
