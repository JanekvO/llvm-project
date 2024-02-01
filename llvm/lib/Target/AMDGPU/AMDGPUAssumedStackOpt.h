//===- AMDGPUAssumedStackOpt.h --cli options for resource analysis-*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// As AMDGPUResourceUsageAnalysis transitions to MC layer propagation of
/// resource info these options are used in multiple files.
///
//===----------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUAssumedStackOpt_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUAssumedStackOpt_H

extern llvm::cl::opt<uint32_t> clAssumedStackSizeForExternalCall;
extern llvm::cl::opt<uint32_t> clAssumedStackSizeForDynamicSizeObjects;

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUAssumedStackOpt_H
