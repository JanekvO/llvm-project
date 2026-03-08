; RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -amdgpu-enable-lower-module-lds=0 -filetype=obj < %s | llvm-readobj -r - | FileCheck %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -amdgpu-enable-lower-module-lds=0 -filetype=asm < %s | FileCheck %s --check-prefix=ASM

; Test that address-taken on a declaration (cross-TU scenario) emits
; address-taken and prototype entries. The function is defined in another TU
; but its address is taken here.

declare void @external_func(i32)

define void @taker(ptr %p) {
  store ptr @external_func, ptr %p
  ret void
}

define amdgpu_kernel void @kern() #0 {
  %p = alloca ptr, addrspace(5)
  call void @taker(ptr addrspace(5) %p)
  ret void
}

attributes #0 = { "amdgpu-link-time-lds" }

; CHECK: .rela.amdgpu.callgraph {
; CHECK-DAG: R_AMDGPU_ABS64 external_func
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_address_taken
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_proto.vi
; CHECK: }

; Assembly output: callgraph section with kernel, call, address-taken, and
; prototype pairs for a cross-TU scenario.
; ASM:           .section .amdgpu.callgraph,"e",@progbits
; ASM-NEXT:      .quad kern
; ASM-NEXT:      .quad kern
; ASM-NEXT:      .quad kern
; ASM-NEXT:      .quad taker
; ASM-NEXT:      .weak __amdgpu_address_taken
; ASM-NEXT:      .quad external_func
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .quad taker
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .weak __amdgpu_proto.vi
; ASM-NEXT:      .quad external_func
; ASM-NEXT:      .quad __amdgpu_proto.vi
; ASM-NEXT:      .weak __amdgpu_proto.vl
; ASM-NEXT:      .quad taker
; ASM-NEXT:      .quad __amdgpu_proto.vl
