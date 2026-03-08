; RUN: opt -S -mtriple=amdgcn-- -amdgpu-enable-object-linking \
; RUN:   -passes=amdgpu-lower-exec-sync,amdgpu-lower-module-lds < %s \
; RUN:   | FileCheck %s

; Verify that with object linking enabled:
; 1. AMDGPULowerExecSync externalizes named barriers and emits
;    amdgpu.callgraph.named_barriers metadata with (barrier, func...) format
; 2. AMDGPULowerModuleLDS does not handle named barriers at all
; 3. amdgpu.callgraph.lds does NOT contain barrier entries

@bar = internal addrspace(3) global target("amdgcn.named.barrier", 0) poison
@lds = internal addrspace(3) global [4 x i32] poison, align 4

; Named barrier becomes an external declaration (externalized by ExecSync).
; CHECK: @bar = external dso_local addrspace(3) global target("amdgcn.named.barrier", 0)
; CHECK-NOT: !absolute_symbol
; Regular LDS is packed into the per-function struct (external, for linker).
; CHECK: @__amdgpu_lds.kernel = external dso_local addrspace(3) global %__amdgpu_lds.kernel.t, align 16

define amdgpu_kernel void @kernel(i32 %idx) {
; CHECK-LABEL: define amdgpu_kernel void @kernel(
; CHECK:         call void @llvm.amdgcn.s.barrier.signal.var(ptr addrspace(3) @bar, i32 3)
; CHECK:         call void @llvm.amdgcn.s.barrier.join(ptr addrspace(3) @bar)
  call void @llvm.amdgcn.s.barrier.signal.var(ptr addrspace(3) @bar, i32 3)
  call void @llvm.amdgcn.s.barrier.join(ptr addrspace(3) @bar)
  call void @llvm.amdgcn.s.barrier.wait(i16 1)
  %gep = getelementptr [4 x i32], ptr addrspace(3) @lds, i32 0, i32 %idx
  store i32 42, ptr addrspace(3) %gep, align 4
  ret void
}

declare void @llvm.amdgcn.s.barrier.signal.var(ptr addrspace(3), i32) #0
declare void @llvm.amdgcn.s.barrier.join(ptr addrspace(3)) #0
declare void @llvm.amdgcn.s.barrier.wait(i16) #0

; Named barrier metadata: (barrier_sym, func1, ...) -- emitted by ExecSync.
; CHECK-DAG: !{ptr addrspace(3) @bar, ptr @kernel}
; LDS struct metadata -- emitted by LDS pass.
; CHECK-DAG: !{ptr @kernel, ptr addrspace(3) @__amdgpu_lds.kernel}

attributes #0 = { convergent nounwind }
