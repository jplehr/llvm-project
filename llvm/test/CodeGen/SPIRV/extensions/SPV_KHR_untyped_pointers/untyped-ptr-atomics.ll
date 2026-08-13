; RUN: llc -O0 -verify-machineinstrs -mtriple=spirv64-unknown-unknown --spirv-ext=+SPV_KHR_untyped_pointers %s -o - | FileCheck %s
; RUN: llc -O0 -verify-machineinstrs -mtriple=spirv64-amd-amdhsa %s -o - | FileCheck %s
; RUN: %if spirv-tools %{ llc -O0 -mtriple=spirv64-unknown-unknown --spirv-ext=+SPV_KHR_untyped_pointers %s -o - -filetype=obj | spirv-val %}

; Atomic load/store/rmw through an untyped pointer.

; CHECK: OpCapability UntypedPointersKHR
; CHECK: OpExtension "SPV_KHR_untyped_pointers"

; CHECK-DAG: %[[#PTR:]] = OpTypeUntypedPointerKHR CrossWorkgroup
; CHECK-DAG: %[[#GENERIC_PTR:]] = OpTypeUntypedPointerKHR Generic
; CHECK-DAG: %[[#I32:]] = OpTypeInt 32 0
; CHECK-DAG: %[[#I64:]] = OpTypeInt 64 0

; CHECK: %[[#P:]] = OpFunctionParameter %[[#PTR]]
; CHECK: OpAtomicStore %[[#P]] %[[#]] %[[#]] %[[#]]
; CHECK: OpAtomicLoad %[[#I32]] %[[#P]] %[[#]] %[[#]]
; CHECK: OpAtomicIAdd %[[#I32]] %[[#P]] %[[#]] %[[#]] %[[#]]
define spir_kernel void @test(ptr addrspace(1) %p) {
  store atomic i32 7, ptr addrspace(1) %p seq_cst, align 4
  %v = load atomic i32, ptr addrspace(1) %p seq_cst, align 4
  %old = atomicrmw add ptr addrspace(1) %p, i32 1 seq_cst
  ret void
}

; Pointer-valued atomics use integer atomics under the physical addressing
; model, even though the pointer values themselves have untyped pointer types.
; CHECK: %[[#PTRPTR:]] = OpFunctionParameter %[[#GENERIC_PTR]]
; CHECK: %[[#VALUE:]] = OpFunctionParameter %[[#GENERIC_PTR]]
; CHECK: %[[#LOAD_PTR:]] = OpBitcast %[[#GENERIC_PTR]] %[[#PTRPTR]]
; CHECK: %[[#LOADED_INT:]] = OpAtomicLoad %[[#I64]] %[[#LOAD_PTR]] %[[#]] %[[#]]
; CHECK: OpConvertUToPtr %[[#GENERIC_PTR]] %[[#LOADED_INT]]
; CHECK: %[[#VALUE_INT:]] = OpConvertPtrToU %[[#I64]] %[[#VALUE]]
; CHECK: %[[#STORE_PTR:]] = OpBitcast %[[#GENERIC_PTR]] %[[#PTRPTR]]
; CHECK: OpAtomicStore %[[#STORE_PTR]] %[[#]] %[[#]] %[[#VALUE_INT]]
; CHECK: %[[#EXCHANGE_VALUE_INT:]] = OpConvertPtrToU %[[#I64]] %[[#VALUE]]
; CHECK: %[[#EXCHANGE_PTR:]] = OpBitcast %[[#GENERIC_PTR]] %[[#PTRPTR]]
; CHECK: %[[#EXCHANGED_INT:]] = OpAtomicExchange %[[#I64]] %[[#EXCHANGE_PTR]] %[[#]] %[[#]] %[[#EXCHANGE_VALUE_INT]]
; CHECK: OpConvertUToPtr %[[#GENERIC_PTR]] %[[#EXCHANGED_INT]]
define spir_func ptr addrspace(4) @test_ptr_atomics(
    ptr addrspace(4) %ptr, ptr addrspace(4) %value) {
  %loaded = load atomic ptr addrspace(4), ptr addrspace(4) %ptr monotonic,
      align 8
  store atomic ptr addrspace(4) %value, ptr addrspace(4) %ptr monotonic,
      align 8
  %exchanged = atomicrmw xchg ptr addrspace(4) %ptr,
      ptr addrspace(4) %value monotonic, align 8
  ret ptr addrspace(4) %loaded
}
