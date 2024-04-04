//===- AMDGPUDelayedMCExpr.h - Delayed MCExpr resolve -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUDELAYEDMCEXPR_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUDELAYEDMCEXPR_H

#include "llvm/BinaryFormat/MsgPackDocument.h"
#include <vector>

namespace llvm {
class MCExpr;

class DelayedMCExpr {
  struct DelayedExpr {
    msgpack::DocNode &DN;
    msgpack::Type Type;
    const MCExpr *Expr;
    DelayedExpr(msgpack::DocNode &DN, msgpack::Type Type, const MCExpr *Expr)
        : DN(DN), Type(Type), Expr(Expr) {}
  };

  std::vector<DelayedExpr> DelayedExprs;

public:
  void ResolveDelayedExpressions();
  void AssignDocNode(msgpack::DocNode &DN, msgpack::Type Type,
                     const MCExpr *Expr);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUDELAYEDMCEXPR_H
