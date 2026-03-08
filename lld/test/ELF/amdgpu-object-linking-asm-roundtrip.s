# REQUIRES: amdgpu

## Test that the object-linking flow round-trips through assembly.
## Compiling with -filetype=asm (as --save-temps does) and then assembling
## with llvm-mc must produce objects that link identically to direct
## -filetype=obj compilation.

# RUN: split-file %s %t

## --- Direct path: llc -filetype=obj -> ld.lld ---
# RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -filetype=obj %t/a.ll -o %t/a.o
# RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -filetype=obj %t/b.ll -o %t/b.o
# RUN: ld.lld %t/a.o %t/b.o -o %t/out_direct
# RUN: llvm-readobj --notes %t/out_direct | FileCheck %s

## --- Round-trip path: llc -filetype=asm -> llvm-mc -> ld.lld ---
# RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -filetype=asm %t/a.ll -o %t/a.s
# RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -filetype=asm %t/b.ll -o %t/b.s
# RUN: llvm-mc -triple=amdgcn-amd-amdhsa -mcpu=gfx900 -filetype=obj %t/a.s -o %t/a_rt.o
# RUN: llvm-mc -triple=amdgcn-amd-amdhsa -mcpu=gfx900 -filetype=obj %t/b.s -o %t/b_rt.o
# RUN: ld.lld %t/a_rt.o %t/b_rt.o -o %t/out_roundtrip
# RUN: llvm-readobj --notes %t/out_roundtrip | FileCheck %s

# CHECK:      .name: kernel
# CHECK:      .sgpr_count:
# CHECK:      .vgpr_count:

#--- a.ll
define amdgpu_kernel void @kernel(i32 %x) {
entry:
  call void @helper()
  ret void
}

declare void @helper()

#--- b.ll
define void @helper() {
entry:
  ret void
}
