#include <stdint.h>

/*
 * Prototype-only mock OpenMP device runtime for amdgcn-flavored SPIR-V.
 *
 * This file exists only to satisfy symbol references during prototype builds.
 * It is intentionally a semantic no-op and must never be used for execution.
 *
 * The stub set covers the current minimal examples:
 *   - #pragma omp target map(...)
 *   - #pragma omp target teams distribute parallel for
 *
 * If the kernel shape changes and new __kmpc_* symbols appear in device IR,
 * extend this file accordingly.
 */

int32_t __kmpc_target_init(void *KernelEnv, void *Ident) {
  (void)KernelEnv;
  (void)Ident;
  return -1;
}

void __kmpc_target_deinit(void) {}

int32_t __kmpc_global_thread_num(void *Loc) {
  (void)Loc;
  return 0;
}

int32_t __kmpc_get_hardware_num_threads_in_block(void) { return 1; }

void __kmpc_distribute_static_init_4(void *Loc, int32_t GTid, int32_t SchedType,
                                     int32_t *PLastIter, int32_t *PLower,
                                     int32_t *PUpper, int32_t *PStride,
                                     int32_t Incr, int32_t Chunk) {
  (void)Loc;
  (void)GTid;
  (void)SchedType;
  (void)Incr;
  (void)Chunk;

  if (PLastIter)
    *PLastIter = 1;
  if (PLower)
    *PLower = 0;
  if (PUpper)
    *PUpper = 0;
  if (PStride)
    *PStride = 1;
}

void __kmpc_distribute_static_fini(void *Loc, int32_t GTid) {
  (void)Loc;
  (void)GTid;
}

void __kmpc_for_static_init_4(void *Loc, int32_t GTid, int32_t SchedType,
                              int32_t *PLastIter, int32_t *PLower,
                              int32_t *PUpper, int32_t *PStride, int32_t Incr,
                              int32_t Chunk) {
  (void)Loc;
  (void)GTid;
  (void)SchedType;
  (void)Incr;
  (void)Chunk;

  if (PLastIter)
    *PLastIter = 1;
  if (PLower)
    *PLower = 0;
  if (PUpper)
    *PUpper = 0;
  if (PStride)
    *PStride = 1;
}

void __kmpc_for_static_fini(void *Loc, int32_t GTid) {
  (void)Loc;
  (void)GTid;
}

void __kmpc_parallel_60(void *Loc, int32_t GTid, int32_t NumThreads,
                        int32_t IfExpr, int32_t ProcBind, void *OutlinedFn,
                        void *WrapperFn, void *CapturedVars, int64_t NumArgs,
                        int32_t Flags) {
  (void)Loc;
  (void)GTid;
  (void)NumThreads;
  (void)IfExpr;
  (void)ProcBind;
  (void)OutlinedFn;
  (void)WrapperFn;
  (void)CapturedVars;
  (void)NumArgs;
  (void)Flags;
}

void __kmpc_fork_call(void *Loc, int32_t NumArgs, void (*OutlineFn)(void),
                      ...) {
  (void)Loc;
  (void)NumArgs;
  (void)OutlineFn;
}

void __kmpc_fork_teams(void *Loc, int32_t NumArgs, void (*OutlineFn)(void),
                       ...) {
  (void)Loc;
  (void)NumArgs;
  (void)OutlineFn;
}
