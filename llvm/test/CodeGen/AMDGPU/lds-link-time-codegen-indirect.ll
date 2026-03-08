; RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -amdgpu-enable-lower-module-lds=0 -filetype=obj < %s | llvm-readobj -r --sections - | FileCheck %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -amdgpu-enable-lower-module-lds=0 -filetype=asm < %s | FileCheck %s --check-prefix=ASM

; Test that the .amdgpu.callgraph section includes address-taken, indirect
; call, and prototype entries for functions involved in indirect calling.
;
; @callee_vi: void(i32) -- address taken, prototype encoding "vi"
; @caller:    has indirect call to void(i32) -- icall encoding "vi"
; @my_kernel: kernel that passes @callee_vi as a function pointer to @caller

define void @callee_vi(i32 %x) {
  ret void
}

define void @caller(ptr %fptr) {
  call void %fptr(i32 1)
  ret void
}

define amdgpu_kernel void @my_kernel() #0 {
  call void @caller(ptr @callee_vi)
  ret void
}

attributes #0 = { "amdgpu-link-time-lds" }

; CHECK:      Section {
; CHECK:        Name: .amdgpu.callgraph
; CHECK:        Type: SHT_PROGBITS
; CHECK:        Flags [
; CHECK:          SHF_EXCLUDE
; CHECK:        ]

; Kernel entry + direct call + address-taken + indirect call + prototype.
; CHECK:      Section {{.*}} .rela.amdgpu.callgraph {
; CHECK-DAG:    R_AMDGPU_ABS64 my_kernel
; CHECK-DAG:    R_AMDGPU_ABS64 caller
; CHECK-DAG:    R_AMDGPU_ABS64 callee_vi
; CHECK-DAG:    R_AMDGPU_ABS64 __amdgpu_address_taken
; CHECK-DAG:    R_AMDGPU_ABS64 __amdgpu_icall.vi
; CHECK-DAG:    R_AMDGPU_ABS64 __amdgpu_proto.vi
; CHECK:      }

; Assembly output: callgraph section with kernel, call, address-taken,
; indirect-call, and prototype pairs.
; ASM:           .section .amdgpu.callgraph,"e",@progbits
; ASM-NEXT:      .quad my_kernel
; ASM-NEXT:      .quad my_kernel
; ASM-NEXT:      .quad my_kernel
; ASM-NEXT:      .quad caller
; ASM-NEXT:      .weak __amdgpu_address_taken
; ASM-NEXT:      .quad callee_vi
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .weak __amdgpu_icall.vi
; ASM-NEXT:      .quad caller
; ASM-NEXT:      .quad __amdgpu_icall.vi
; ASM-NEXT:      .weak __amdgpu_proto.vi
; ASM-NEXT:      .quad callee_vi
; ASM-NEXT:      .quad __amdgpu_proto.vi
