//===- AMDGPUAssumedStackOpt.h --cli options for resource analysis-----------=//
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
#include "AMDGPUAssumedStackOpt.h"

// In code object v4 and older, we need to tell the runtime some amount ahead of
// time if we don't know the true stack size. Assume a smaller number if this is
// only due to dynamic / non-entry block allocas.
llvm::cl::opt<uint32_t> clAssumedStackSizeForExternalCall(
    "amdgpu-assume-external-call-stack-size",
    llvm::cl::desc("Assumed stack use of any external call (in bytes)"),
    llvm::cl::Hidden, llvm::cl::init(16384));

llvm::cl::opt<uint32_t> clAssumedStackSizeForDynamicSizeObjects(
    "amdgpu-assume-dynamic-stack-object-size",
    llvm::cl::desc("Assumed extra stack use if there are any "
                   "variable sized objects (in bytes)"),
    llvm::cl::Hidden, llvm::cl::init(4096));
