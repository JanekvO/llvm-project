//===-- AMDGPUAsmPrinter.h - Print AMDGPU assembly code ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// AMDGPU Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUASMPRINTER_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUASMPRINTER_H

#include "AMDGPUResourceUsageAnalysis.h"
#include "MCTargetDesc/AMDGPUMCExpr.h"
#include "SIProgramInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"

struct amd_kernel_code_t;

namespace llvm {

class AMDGPUMachineFunction;
struct AMDGPUResourceUsageAnalysis;
class AMDGPUTargetStreamer;
class MCCodeEmitter;
class MCOperand;

namespace AMDGPU {
namespace HSAMD {
class MetadataStreamer;
}
} // namespace AMDGPU

namespace amdhsa {
struct kernel_descriptor_t;
}

namespace {
class ResourceInfo {
public:
  enum ResourceInfoKind {
    RIK_NumVGPR,
    RIK_NumAGPR,
    RIK_NumSGPR,
    RIK_TotalNumVGPR,
    RIK_TotalNumSGPR,
    RIK_PrivateSegSize,
    RIK_UsesVCC,
    RIK_UsesFlatScratch,
    RIK_HasDynSizedStack,
    RIK_HasRecursion,
    RIK_HasIndirectCall
  };

private:
  int32_t MaxVGPR;
  int32_t MaxAGPR;
  int32_t MaxSGPR;

  MCContext &OutContext;
  bool finalized;

  void assignResourceInfoExpr(int64_t localValue, ResourceInfoKind RIK,
                              AMDGPUMCExpr::AMDGPUExprKind Kind,
                              const MachineFunction &MF,
                              const SmallVectorImpl<const Function *> &Callees);

  int64_t getTotalNumVGPRs(StringRef FuncName, int64_t NumAGPR,
                           int64_t NumVGPR);
  int64_t getTotalNumSGPRs(StringRef FuncName, int64_t NumExplicitSGPR,
                           bool UsesVCC, bool UsesFlatScr);

  // Set members that require MF information that is unavailable at the time
  // some symbols require resolving.
  void Prepare(const MachineFunction &MF);

  // These are derived from IsaInfo::getNumExtraSGPRs.
  enum ExtraSGPRKind {
    V8Min,
    V8ToV10FeatureArchFlatScr,
    V8ToV10XNACKUsed,
    V8ToV10,
    V10Plus
  };

  // Mappings required to compute the finalized total register usage.
  StringMap<bool> has90AInsts;
  StringMap<ExtraSGPRKind> ExtraSGPRType;

  std::vector<StringRef> FunctionNames;
  void emitComments(StringRef FuncName, raw_ostream &OS);

  // Assigns expression for Max S/V/A-GPRs to the referenced symbols.
  void assignMaxRegs();

  // Assigns expressions for total S/V-GPR to the referenced symbols.
  void assignTotalRegs();

public:
  ResourceInfo(MCContext &OutContext)
      : MaxVGPR(0), MaxAGPR(0), MaxSGPR(0), OutContext(OutContext),
        finalized(false) {}
  void addMaxVGPRCandidate(int32_t candidate) {
    MaxVGPR = std::max(MaxVGPR, candidate);
  }
  void addMaxAGPRCandidate(int32_t candidate) {
    MaxAGPR = std::max(MaxAGPR, candidate);
  }
  void addMaxSGPRCandidate(int32_t candidate) {
    MaxSGPR = std::max(MaxAGPR, candidate);
  }

  MCSymbol *getSymbol(StringRef FuncName, ResourceInfoKind RIK);

  // Resolves the final symbols that requires the inter-function resource info
  // to be resolved.
  void Finalize();

  MCSymbol *getMaxVGPRSymbol();
  MCSymbol *getMaxAGPRSymbol();
  MCSymbol *getMaxSGPRSymbol();

  /// AMDGPUResourceUsageAnalysis gathers resource usage on a per-function
  /// granularity. However, some resource info has to be assigned the call
  /// transitive maximum or accumulative. For example, if A calls B and B's VGPR
  /// usage exceeds A's, A should be assigned B's VGPR usage. Furthermore,
  /// functions with indirect calls should be assigned the module level maximum.
  void gatherResourceInfo(
      const MachineFunction &MF,
      const AMDGPUResourceUsageAnalysis::FunctionResourceInfo &FRI);

  // Previously, the comments were emitted for every function, at every
  // function. However, with the move to MCExprs it may not be a solvable
  // expression at that time so if it is requested, the expr should be emitted
  // at the end when everything should be solved (and if not, it's a bigger
  // issue at hand).
  void emitAllComments(raw_ostream &OS);
};
} // end anonymous namespace

class AMDGPUAsmPrinter final : public AsmPrinter {
private:
  unsigned CodeObjectVersion;
  void initializeTargetID(const Module &M);

  AMDGPUResourceUsageAnalysis *ResourceUsage;

  std::unique_ptr<ResourceInfo> RI;

  SIProgramInfo CurrentProgramInfo;

  std::unique_ptr<AMDGPU::HSAMD::MetadataStreamer> HSAMetadataStream;

  MCCodeEmitter *DumpCodeInstEmitter = nullptr;

  uint64_t getFunctionCodeSize(const MachineFunction &MF) const;

  void getSIProgramInfo(SIProgramInfo &Out, const MachineFunction &MF);
  void getAmdKernelCode(amd_kernel_code_t &Out, const SIProgramInfo &KernelInfo,
                        const MachineFunction &MF) const;

  /// Emit register usage information so that the GPU driver
  /// can correctly setup the GPU state.
  void EmitProgramInfoSI(const MachineFunction &MF,
                         const SIProgramInfo &KernelInfo);
  void EmitPALMetadata(const MachineFunction &MF,
                       const SIProgramInfo &KernelInfo);
  void emitPALFunctionMetadata(const MachineFunction &MF);
  void emitCommonFunctionComments(uint32_t NumVGPR,
                                  std::optional<uint32_t> NumAGPR,
                                  uint32_t TotalNumVGPR, uint32_t NumSGPR,
                                  uint64_t ScratchSize, uint64_t CodeSize,
                                  const AMDGPUMachineFunction *MFI);
  void emitResourceUsageRemarks(const MachineFunction &MF,
                                const SIProgramInfo &CurrentProgramInfo,
                                bool isModuleEntryFunction, bool hasMAIInsts);

  uint16_t getAmdhsaKernelCodeProperties(
      const MachineFunction &MF) const;

  amdhsa::kernel_descriptor_t getAmdhsaKernelDescriptor(
      const MachineFunction &MF,
      const SIProgramInfo &PI) const;

  void initTargetStreamer(Module &M);

public:
  explicit AMDGPUAsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer);

  StringRef getPassName() const override;

  const MCSubtargetInfo* getGlobalSTI() const;

  AMDGPUTargetStreamer* getTargetStreamer() const;

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;
  bool runOnMachineFunction(MachineFunction &MF) override;

  /// Wrapper for MCInstLowering.lowerOperand() for the tblgen'erated
  /// pseudo lowering.
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const;

  /// Lower the specified LLVM Constant to an MCExpr.
  /// The AsmPrinter::lowerConstantof does not know how to lower
  /// addrspacecast, therefore they should be lowered by this function.
  const MCExpr *lowerConstant(const Constant *CV) override;

  /// tblgen'erated driver function for lowering simple MI->MC pseudo
  /// instructions.
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);


  /// Implemented in AMDGPUMCInstLower.cpp
  void emitInstruction(const MachineInstr *MI) override;

  void emitFunctionBodyStart() override;

  void emitFunctionBodyEnd() override;

  void emitImplicitDef(const MachineInstr *MI) const override;

  void emitFunctionEntryLabel() override;

  void emitBasicBlockStart(const MachineBasicBlock &MBB) override;

  void emitGlobalVariable(const GlobalVariable *GV) override;

  void emitStartOfAsmFile(Module &M) override;

  void emitEndOfAsmFile(Module &M) override;

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;

  int32_t MaxVGPR;
  int32_t MaxAGPR;
  int32_t MaxSGPR;

protected:
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  std::vector<std::string> DisasmLines, HexLines;
  size_t DisasmLineMaxLen;
  bool IsTargetStreamerInitialized;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUASMPRINTER_H
