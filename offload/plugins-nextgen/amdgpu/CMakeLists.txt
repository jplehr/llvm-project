################################################################################
set(LIBOMPTARGET_BUILD_AMDGPU_PLUGIN TRUE CACHE BOOL
  "Whether to build AMDGPU plugin")
if (NOT LIBOMPTARGET_BUILD_AMDGPU_PLUGIN)
  message(STATUS "Not building AMDGPU NextGen offloading plugin: LIBOMPTARGET_BUILD_AMDGPU_PLUGIN is false")
  return()
endif()

# If we are bootstrapping hsa via external project we need to use find_library
# as it will not be installed.
if(DEFINED LIBOMPTARGET_EXTERNAL_PROJECT_HSA_PATH)
  if(OFFLOAD_EXTERNAL_PROJECT_UNIFIED_ROCR)
    find_library(HSA_RUNTIME hsa-runtime64
      HINTS
      ${CMAKE_BINARY_DIR}/../../runtimes/rocr-runtime-prefix/src/rocr-runtime-build/rocr/lib
      ${CMAKE_INSTALL_PREFIX}
      PATHS
      /opt/rocm)
  else()
    find_library(HSA_RUNTIME hsa-runtime64
      HINTS
      ${CMAKE_BINARY_DIR}/../../runtimes/rocr-runtime-prefix/src/rocr-runtime-build
      ${CMAKE_INSTALL_PREFIX}
      PATHS
      ${LIBOMPTARGET_EXTERNAL_PROJECT_HSA_PATH})
  endif()
else()
  # As of rocm-3.7, hsa is installed with cmake packages and kmt is found via hsa
  find_package(hsa-runtime64 QUIET 1.2.0 HINTS ${CMAKE_INSTALL_PREFIX} PATHS /opt/rocm)
endif()

# Create the library and add the default arguments.
add_target_library(omptarget.rtl.amdgpu AMDGPU)

target_sources(omptarget.rtl.amdgpu PRIVATE src/rtl.cpp src/ComgrAdapter.cpp)
target_include_directories(omptarget.rtl.amdgpu PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/utils)
if(DEFINED LIBOMPTARGET_EXTERNAL_PROJECT_HSA_PATH)
  target_include_directories(omptarget.rtl.amdgpu PRIVATE
    ${LIBOMPTARGET_EXTERNAL_PROJECT_HSA_PATH}/runtime/hsa-runtime/inc )
endif()
target_include_directories(omptarget.rtl.amdgpu PRIVATE
  ${CMAKE_INSTALL_PREFIX}/include)

if (LLVM_LINK_LLVM_DYLIB)
  set(llvm_libs LLVM)
else()
  set(llvm_libs LLVMFrontendOffloading)
endif()

option(LIBOMPTARGET_BUILD_AMDGPU_COMGR_SUPPORT
  "Enable COMGR-backed SPIR-V runtime JIT path in AMDGPU plugin" ON)
option(LIBOMPTARGET_FORCE_DLOPEN_LIBCOMGR
  "Build AMDGPU plugin with dlopened COMGR runtime" OFF)

if (LIBOMPTARGET_BUILD_AMDGPU_COMGR_SUPPORT)
  find_library(AMD_COMGR_RUNTIME amd_comgr
    HINTS
      ${CMAKE_INSTALL_PREFIX}/lib
      ${CMAKE_INSTALL_PREFIX}/lib64
    PATHS
      /opt/rocm/lib
      /opt/rocm/lib64)

  if (AMD_COMGR_RUNTIME AND NOT LIBOMPTARGET_FORCE_DLOPEN_LIBCOMGR)
    message(STATUS "Building AMDGPU plugin with linked COMGR runtime")
    target_link_libraries(omptarget.rtl.amdgpu PRIVATE ${AMD_COMGR_RUNTIME})
    target_compile_definitions(omptarget.rtl.amdgpu PRIVATE
      LIBOMPTARGET_AMDGPU_COMGR_LINKED)
  else()
    message(STATUS "Building AMDGPU plugin with dlopened COMGR runtime")
    target_compile_definitions(omptarget.rtl.amdgpu PRIVATE
      LIBOMPTARGET_AMDGPU_COMGR_DYNAMIC)
  endif()
else()
  message(STATUS "Building AMDGPU plugin with COMGR disabled")
  target_compile_definitions(omptarget.rtl.amdgpu PRIVATE
    LIBOMPTARGET_AMDGPU_COMGR_DISABLED)
endif()

option(LIBOMPTARGET_FORCE_DLOPEN_LIBHSA "Build with dlopened libhsa" OFF)
if(DEFINED HSA_RUNTIME AND NOT LIBOMPTARGET_FORCE_DLOPEN_LIBHSA)
  message(STATUS "Building AMDGPU plugin linked against libhsa")
  target_link_libraries(omptarget.rtl.amdgpu PRIVATE ${HSA_RUNTIME})
  get_filename_component(library_path ${HSA_RUNTIME} DIRECTORY)
  target_include_directories(omptarget.rtl.amdgpu PRIVATE ${library_path}/../include)
elseif(hsa-runtime64_FOUND AND NOT LIBOMPTARGET_FORCE_DLOPEN_LIBHSA)
  message(STATUS "Building AMDGPU plugin linked against libhsa")
  target_link_libraries(omptarget.rtl.amdgpu PRIVATE hsa-runtime64::hsa-runtime64 ${llvm_libs})
else()
  message(STATUS "Building AMDGPU plugin for dlopened libhsa")
  target_include_directories(omptarget.rtl.amdgpu PRIVATE dynamic_hsa)
  target_sources(omptarget.rtl.amdgpu PRIVATE dynamic_hsa/hsa.cpp)
  target_link_libraries(omptarget.rtl.amdgpu PRIVATE ${llvm_libs})
endif()

# Configure testing for the AMDGPU plugin. We will build tests if we could a
# functional AMD GPU on the system, or if manually specifies by the user.
option(LIBOMPTARGET_FORCE_AMDGPU_TESTS "Build AMDGPU libomptarget tests" OFF)
if (LIBOMPTARGET_FOUND_AMDGPU_GPU OR LIBOMPTARGET_FORCE_AMDGPU_TESTS)
  # Report to the parent scope that we are building a plugin for amdgpu
  set(LIBOMPTARGET_SYSTEM_TARGETS
      "${LIBOMPTARGET_SYSTEM_TARGETS} amdgcn-amd-amdhsa" PARENT_SCOPE)
  list(APPEND LIBOMPTARGET_TESTED_PLUGINS "omptarget.rtl.amdgpu")
  set(LIBOMPTARGET_TESTED_PLUGINS "${LIBOMPTARGET_TESTED_PLUGINS}" PARENT_SCOPE)
else()
  message(STATUS "Not generating AMDGPU tests, no supported devices detected."
                   " Use 'LIBOMPTARGET_FORCE_AMDGPU_TESTS' to override.")
endif()
