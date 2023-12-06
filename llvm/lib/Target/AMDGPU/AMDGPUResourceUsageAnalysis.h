//===- AMDGPUResourceUsageAnalysis.h ---- analysis of resources -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Analyzes how many registers and other resources are used by
/// functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPURESOURCEUSAGEANALYSIS_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPURESOURCEUSAGEANALYSIS_H

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

class GCNSubtarget;
class MachineFunction;
class TargetMachine;

struct AMDGPUResourceUsageAnalysis : public MachineFunctionPass {
  static char ID;

public:
  // Track resource usage for callee functions.
  struct FunctionResourceInfo {
    // Track the number of explicitly used VGPRs. Special registers reserved at
    // the end are tracked separately.
    int32_t NumVGPR = 0;
    int32_t NumAGPR = 0;
    int32_t NumExplicitSGPR = 0;
    uint64_t PrivateSegmentSize = 0;
    bool UsesVCC = false;
    bool UsesFlatScratch = false;
    bool HasDynamicallySizedStack = false;
    bool HasRecursion = false;
    bool HasIndirectCall = false;

    SmallVector<const Function *, 16> Callees;

    int32_t getTotalNumSGPRs(const GCNSubtarget &ST) const;
    // Total number of VGPRs is actually a combination of AGPR and VGPR
    // depending on architecture - and some alignment constraints
    int32_t getTotalNumVGPRs(const GCNSubtarget &ST, int32_t NumAGPR,
                             int32_t NumVGPR) const;
    int32_t getTotalNumVGPRs(const GCNSubtarget &ST) const;
  };

  AMDGPUResourceUsageAnalysis() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  const FunctionResourceInfo &getResourceInfo() const {
    return ResourceInfo;
  }

private:
  FunctionResourceInfo analyzeResourceUsage(const MachineFunction &MF) const;
  FunctionResourceInfo ResourceInfo;
};
} // namespace llvm
#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPURESOURCEUSAGEANALYSIS_H
