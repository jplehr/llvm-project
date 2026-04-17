#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${SCRIPT_DIR}/mock-libomptarget-spirv-runtime.c"

OUT_DIR="${1:-/home/janplehr/work1/tickets/lcompiler-spirv-in-openmp/artifacts}"
OUT_BC="${OUT_DIR}/libomptarget-spirv.bc"

CLANG_BIN="${CLANG:-/home/janplehr/rocm/aomp/lib/llvm/bin/clang}"

mkdir -p "${OUT_DIR}"

"${CLANG_BIN}" \
  --target=spirv64-amd-amdhsa \
  -emit-llvm \
  -c \
  -O0 \
  "${SRC}" \
  -o "${OUT_BC}"

echo "Wrote ${OUT_BC}"
