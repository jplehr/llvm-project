# OpenMP AMD-SPIRV Prototype Runtime

This directory contains a prototype-only mock device runtime for the
`spirv64-amd-amdhsa` OpenMP path.

The current prototype strategy is to provide required `__kmpc_*` symbols at
device link time (instead of relying on deferred JIT/load-time resolution).
This keeps failures local to the compile/link flow and mirrors HIP's stricter
linking model more closely.

## Contents

- `mock-libomptarget-spirv-runtime.c`
  - minimal symbol stubs required by the current tiny prototype kernel shape
- `build-mock-runtime.sh`
  - helper script that emits `libomptarget-spirv.bc` into the ticket artifacts
    location used by `SPIRVOpenMPToolChain` prototype fallback

## Build mock runtime bitcode

```bash
offload/libomptarget/prototype/spirv-openmp/build-mock-runtime.sh
```

By default this writes:

`/home/janplehr/work1/tickets/lcompiler-spirv-in-openmp/artifacts/libomptarget-spirv.bc`

Set `CLANG` to override the compiler executable if needed.
Default compiler is the locally installed AOMP clang at
`/home/janplehr/rocm/aomp/lib/llvm/bin/clang`.
