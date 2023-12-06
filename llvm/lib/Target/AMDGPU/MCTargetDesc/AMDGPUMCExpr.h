//===- AMDGPUMCExpr.h - AMDGPU specific MC expression classes ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUMCEXPR_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUMCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class AMDGPUMCExpr : public MCTargetExpr {
public:
  enum AMDGPUExprKind {
    AGEK_None,
    AGEK_ResInfoDirOr,
    AGEK_ResInfoDirMax
  };

private:
  AMDGPUExprKind Kind;
  std::vector<const MCExpr *> Args;

  AMDGPUMCExpr(AMDGPUExprKind Kind, const std::vector<const MCExpr *> &Args)
    : Kind(Kind), Args(Args) {}

public:
  static const AMDGPUMCExpr *create(AMDGPUExprKind Kind, const std::vector<const MCExpr *> &Args, MCContext &Ctx) {
    (void)Ctx;
    return new AMDGPUMCExpr(Kind, Args);
  }

  static const AMDGPUMCExpr *createOr(const std::vector<const MCExpr *> &Args, MCContext &Ctx) {
    return create(AMDGPUExprKind::AGEK_ResInfoDirOr, Args, Ctx);
  }

  static const AMDGPUMCExpr *createMax(const std::vector<const MCExpr *> &Args, MCContext &Ctx) {
    return create(AMDGPUExprKind::AGEK_ResInfoDirMax, Args, Ctx);
  }

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  AMDGPUExprKind getKind() const { return Kind; }
  const MCExpr *getSubExpr(size_t index) const;

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override;
  void fixELFSymbolsInTLSFixups(MCAssembler &) const override;
};

} // end namespace llvm

#endif