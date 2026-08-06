// REQUIRES: x86-registered-target
// REQUIRES: spirv-registered-target

//=============================================================================
// Test 1: Basic compilation with spirv64
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64 -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SPIRV64

// CHECK-SPIRV64: "-cc1" "-triple" "spirv64"
// CHECK-SPIRV64-SAME: "-fopenmp"
// CHECK-SPIRV64: "-cc1" "-triple" "x86_64
// CHECK-SPIRV64-SAME: "-fopenmp"

//=============================================================================
// Test 2: Compilation with spirv64-amd-amdhsa
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64-amd-amdhsa -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-SPIRV64-AMD

// CHECK-SPIRV64-AMD: "-cc1" "-triple" "spirv64-amd-amdhsa"
// CHECK-SPIRV64-AMD-SAME: "-fopenmp"

//=============================================================================
// Test 3: Compilation with --offload-arch=amdgcnspirv
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   --offload-arch=amdgcnspirv -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-AMDGCNSPIRV

// CHECK-AMDGCNSPIRV: "-cc1" "-triple" "spirv64-amd-amdhsa"
// CHECK-AMDGCNSPIRV-SAME: "-fopenmp"

//=============================================================================
// Test 4: Vectorization must be disabled
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64 -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-NOVECTORIZE

// CHECK-NOVECTORIZE: "-cc1" "-triple" "spirv64"
// CHECK-NOVECTORIZE-SAME: "-mllvm" "-vectorize-loops=false"
// CHECK-NOVECTORIZE-SAME: "-mllvm" "-vectorize-slp=false"

//=============================================================================
// Test 5: Hidden visibility must be default
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64 -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-VISIBILITY

// CHECK-VISIBILITY: "-cc1" "-triple" "spirv64"
// CHECK-VISIBILITY-SAME: "-fvisibility=hidden"
// CHECK-VISIBILITY-SAME: "-fapply-global-visibility-to-externs"

//=============================================================================
// Test 6: Device library is found with sysroot
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64 \
// RUN:   --sysroot=%S/Inputs/spirv-openmp %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-DEVLIB

// CHECK-DEVLIB: "-mlink-builtin-bitcode" "{{.*}}libomptarget-spirv.bc"
// CHECK-DEVLIB-SAME: "-mlink-builtin-bitcode" "{{.*}}libc.bc"


//=============================================================================
// Test 7: -nogpulib suppresses device library
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64 -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-NOGPULIB

// CHECK-NOGPULIB-NOT: libomptarget-spirv.bc

//=============================================================================
// Test 8: ROCm path is accepted
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   -fopenmp-targets=spirv64 --rocm-path=/opt/rocm \
// RUN:   -nogpulib %s 2>&1 | FileCheck %s --check-prefix=CHECK-ROCM

// CHECK-ROCM: "-triple" "spirv64"

//=============================================================================
// Test 9: AMD OpenMP SPIR-V keeps device bitcode for wrapper link
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   --offload-arch=amdgcnspirv -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-AMDGCNSPIRV-BC

// CHECK-AMDGCNSPIRV-BC: "-cc1" "-triple" "spirv64-amd-amdhsa"
// CHECK-AMDGCNSPIRV-BC-SAME: "-emit-llvm-bc"
// CHECK-AMDGCNSPIRV-BC: "--image=file={{.*}}.bc,triple=spirv64-amd-amdhsa,arch=amdgcnspirv,kind=openmp"

// RUN: %clang -ccc-print-phases --target=x86_64-linux-gnu -fopenmp \
// RUN:   --offload-arch=amdgcnspirv -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-AMDGCNSPIRV-PHASES

// CHECK-AMDGCNSPIRV-PHASES: compiler, {{.*}} (device-openmp, amdgcnspirv)
// CHECK-AMDGCNSPIRV-PHASES: offload, {{.*}} "device-openmp (spirv64-amd-amdhsa:amdgcnspirv)"
// CHECK-AMDGCNSPIRV-PHASES: llvm-offload-binary
// CHECK-AMDGCNSPIRV-PHASES-NOT: backend, {{.*}} (device-openmp, amdgcnspirv)
// CHECK-AMDGCNSPIRV-PHASES-NOT: assembler, {{.*}} (device-openmp, amdgcnspirv)

//=============================================================================
// Test 10: Route selection flags are forwarded to wrapper device compiler
//=============================================================================

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   --offload-arch=amdgcnspirv -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-USE-SPIRV-BACKEND-DEFAULT

// CHECK-USE-SPIRV-BACKEND-DEFAULT: clang-linker-wrapper
// CHECK-USE-SPIRV-BACKEND-DEFAULT: "--device-compiler=spirv64-amd-amdhsa=-use-spirv-backend"

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   --offload-arch=amdgcnspirv -no-use-spirv-backend -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-NO-SPIRV-BACKEND-FWD

// CHECK-NO-SPIRV-BACKEND-FWD: clang-linker-wrapper
// CHECK-NO-SPIRV-BACKEND-FWD: "--device-compiler=spirv64-amd-amdhsa=-no-use-spirv-backend"

// RUN: %clang -### --target=x86_64-linux-gnu -fopenmp \
// RUN:   --offload-arch=amdgcnspirv -use-spirv-backend -nogpulib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CHECK-USE-SPIRV-BACKEND-FWD

// CHECK-USE-SPIRV-BACKEND-FWD: clang-linker-wrapper
// CHECK-USE-SPIRV-BACKEND-FWD: "--device-compiler=spirv64-amd-amdhsa=-use-spirv-backend"

//=============================================================================
// Test code
//=============================================================================

int main() {
  int a[100];
#pragma omp target teams distribute parallel for
  for (int i = 0; i < 100; i++)
    a[i] = i;
  return a[0];
}
