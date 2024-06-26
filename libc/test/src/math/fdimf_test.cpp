//===-- Unittests for fdimf -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FDimTest.h"

#include "hdr/math_macros.h"
#include "src/__support/FPUtil/FPBits.h"
#include "src/math/fdimf.h"
#include "test/UnitTest/FPMatcher.h"
#include "test/UnitTest/Test.h"

using LlvmLibcFDimTest = FDimTestTemplate<float>;

TEST_F(LlvmLibcFDimTest, NaNArg_fdimf) {
  test_na_n_arg(&LIBC_NAMESPACE::fdimf);
}

TEST_F(LlvmLibcFDimTest, InfArg_fdimf) { test_inf_arg(&LIBC_NAMESPACE::fdimf); }

TEST_F(LlvmLibcFDimTest, NegInfArg_fdimf) {
  test_neg_inf_arg(&LIBC_NAMESPACE::fdimf);
}

TEST_F(LlvmLibcFDimTest, BothZero_fdimf) {
  test_both_zero(&LIBC_NAMESPACE::fdimf);
}

TEST_F(LlvmLibcFDimTest, InFloatRange_fdimf) {
  test_in_range(&LIBC_NAMESPACE::fdimf);
}
