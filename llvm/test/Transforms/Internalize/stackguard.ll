; __stack_chk_guard and __stack_chk_fail should not be internalized.
; RUN: opt < %s -passes=internalize -S | FileCheck %s
; RUN: opt -mtriple=powerpc64-ibm-aix-xcoff < %s -passes=internalize -S | FileCheck %s --check-prefix=AIX
; RUN: opt -mtriple=spirv64-amd-amdhsa < %s -passes=internalize -S | FileCheck %s --check-prefix=SPIRV

; CHECK: @__stack_chk_guard = hidden global [8 x i64] zeroinitializer, align 16
; AIX: @__stack_chk_guard = internal global [8 x i64] zeroinitializer, align 16
@__stack_chk_guard = hidden global [8 x i64] zeroinitializer, align 16

; CHECK: @__stack_chk_fail = hidden global [8 x i64] zeroinitializer, align 16
; AIX: @__stack_chk_fail = hidden global [8 x i64] zeroinitializer, align 16
@__stack_chk_fail = hidden global [8 x i64] zeroinitializer, align 16

; AIX: @__ssp_canary_word = hidden global [8 x i64] zeroinitializer, align 16
@__ssp_canary_word = hidden global [8 x i64] zeroinitializer, align 16

; SPIRV: @__llvm_rpc_client = protected addrspace(1) global i64 0, align 8
@__llvm_rpc_client = protected addrspace(1) global i64 zeroinitializer, align 8
