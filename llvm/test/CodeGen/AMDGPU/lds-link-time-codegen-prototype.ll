; RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -amdgpu-enable-lower-module-lds=0 -filetype=obj < %s | llvm-readobj -r - | FileCheck %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa -mcpu=gfx900 -amdgpu-enable-object-linking -amdgpu-enable-lower-module-lds=0 -filetype=asm < %s | FileCheck %s --check-prefix=ASM

; Test ABI register-size type ID generation for various function signatures.
; The type ID encodes each parameter/return by bit width: v=void, i=<=32-bit,
; l=33-64-bit. Types with the same register footprint share an encoding
; (e.g. float(float) and i32(i32) both produce "ii").

define void @void_void() {
  ret void
}

define i32 @i32_i32(i32 %x) {
  ret i32 %x
}

define void @void_ptr_i32(ptr %p, i32 %x) {
  ret void
}

define i64 @i64_i64_i64(i64 %a, i64 %b) {
  ret i64 %a
}

define float @float_float(float %x) {
  ret float %x
}

; Take the address of each function so they appear in the prototype metadata.
define void @taker() {
  %p0 = alloca ptr, addrspace(5)
  store volatile ptr @void_void, ptr addrspace(5) %p0
  store volatile ptr @i32_i32, ptr addrspace(5) %p0
  store volatile ptr @void_ptr_i32, ptr addrspace(5) %p0
  store volatile ptr @i64_i64_i64, ptr addrspace(5) %p0
  store volatile ptr @float_float, ptr addrspace(5) %p0
  ret void
}

define amdgpu_kernel void @kern() #0 {
  call void @taker()
  ret void
}

attributes #0 = { "amdgpu-link-time-lds" }

; Five address-taken functions produce prototype entries. float(float) and
; i32(i32) share encoding "ii"; the other three are distinct.
; CHECK: .rela.amdgpu.callgraph {
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_proto.v
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_proto.ii
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_proto.vli
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_proto.lll
; CHECK-DAG: R_AMDGPU_ABS64 __amdgpu_address_taken
; CHECK: }

; Assembly output: callgraph section with kernel, call, address-taken, and
; prototype pairs. float(float) and i32(i32) share encoding "ii".
; ASM:           .section .amdgpu.callgraph,"e",@progbits
; ASM-NEXT:      .quad kern
; ASM-NEXT:      .quad kern
; ASM-NEXT:      .quad kern
; ASM-NEXT:      .quad taker
; ASM-NEXT:      .weak __amdgpu_address_taken
; ASM-NEXT:      .quad void_void
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .quad i32_i32
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .quad void_ptr_i32
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .quad i64_i64_i64
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .quad float_float
; ASM-NEXT:      .quad __amdgpu_address_taken
; ASM-NEXT:      .weak __amdgpu_proto.v
; ASM-NEXT:      .quad void_void
; ASM-NEXT:      .quad __amdgpu_proto.v
; ASM-NEXT:      .weak __amdgpu_proto.ii
; ASM-NEXT:      .quad i32_i32
; ASM-NEXT:      .quad __amdgpu_proto.ii
; ASM-NEXT:      .weak __amdgpu_proto.vli
; ASM-NEXT:      .quad void_ptr_i32
; ASM-NEXT:      .quad __amdgpu_proto.vli
; ASM-NEXT:      .weak __amdgpu_proto.lll
; ASM-NEXT:      .quad i64_i64_i64
; ASM-NEXT:      .quad __amdgpu_proto.lll
; ASM-NEXT:      .weak __amdgpu_proto.ii
; ASM-NEXT:      .quad float_float
; ASM-NEXT:      .quad __amdgpu_proto.ii
