//===- AMDGPUMCExpr.cpp - AMDGPU specific MC expression classes -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AMDGPUMCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

const MCExpr *AMDGPUMCExpr::getSubExpr(size_t index) const {
  assert(index < Args.size() && "Indexing out of bounds AMDGPUMCExpr sub-expr");
  return Args[index];
}

void AMDGPUMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  // No need to encapsulate if there's only 1 arg.
  if (Args.size() == 1) {
    assert(Args[0]->getKind() == ExprKind::Constant && "AMDGPUMCExpr should be a constant if there is only 1 arg.");
    Args[0]->print(OS, MAI, false);
    return;
  }

  switch(Kind) {
    default: llvm_unreachable("AMDGPUMCExpr kind does not exist");
    case AGEK_ResInfoDirOr:
      OS << "OR(";
      break;
    case AGEK_ResInfoDirMax:
      OS << "MAX(";
      break;
  }
  for (auto it = Args.begin(); it != Args.end(); ++it) {
      (*it)->print(OS, MAI, true);
    if ((it+1) != Args.end())
      OS << ", ";
  }
  OS << ")";
}

bool AMDGPUMCExpr::evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const {
  int64_t total = 0;
  auto Op = [this](int64_t Arg1, int64_t Arg2) -> int64_t {
    switch (this->Kind) {
    default: llvm_unreachable("Unknown AMDGPUMCExpr kind.");
    case AGEK_ResInfoDirMax:
      return Arg1 > Arg2 ? Arg1 : Arg2;
    case AGEK_ResInfoDirOr:
      return Arg1 | Arg2;
    }
  };

  for (const MCExpr *Arg : Args) {
    if (!Arg->evaluateAsRelocatable(Res, Layout, Fixup))
      return false;
    if (!Res.isAbsolute())
      return false;
    total = Op(total, Res.getConstant());
  }

  Res = MCValue::get(total);
  return true;
}

void AMDGPUMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  for (const MCExpr *Arg : Args) {
    Streamer.visitUsedExpr(*Arg);
  }
}

MCFragment *AMDGPUMCExpr::findAssociatedFragment() const { return getSubExpr(0)->findAssociatedFragment(); }
void AMDGPUMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &MCAsm) const {}
