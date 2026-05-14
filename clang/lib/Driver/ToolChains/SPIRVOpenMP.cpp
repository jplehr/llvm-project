//===- SPIRVOpenMP.cpp - SPIR-V OpenMP ToolChain -*- C++ -*-----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPIRVOpenMP.h"
#include "clang/Driver/CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Options/Options.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace llvm::opt;

namespace clang::driver::tools::SPIRVOpenMP {

void Linker::constructLinkAndEmitSpirvCommand(
    Compilation &C, const JobAction &JA, const InputInfoList &Inputs,
    const InputInfo &Output, const llvm::opt::ArgList &Args) const {
  assert(!Inputs.empty() && "Must have at least one input.");

  const auto &TC =
      static_cast<const toolchains::SPIRVOpenMPToolChain &>(getToolChain());
  StringRef OutputFileName = Output.getFilename();

  std::string TempBCName = C.getDriver().GetTemporaryPath(
      llvm::sys::path::stem(OutputFileName), "bc");
  const char *TempFile = C.getArgs().MakeArgString(TempBCName);

  ArgStringList LinkArgs;
  for (const auto &Input : Inputs)
    if (Input.isFilename())
      LinkArgs.push_back(Input.getFilename());

  for (const auto &BCLib : TC.getDeviceLibs(Args, Action::OFK_OpenMP))
    LinkArgs.push_back(Args.MakeArgString(BCLib.Path));

  for (const Arg *A : Args.filtered(options::OPT_mlink_builtin_bitcode))
    LinkArgs.push_back(A->getValue());

  tools::constructLLVMLinkCommand(C, *this, JA, Inputs, LinkArgs, Output, Args,
                                  TempFile);

  bool UseSPIRVBackend =
      Args.hasFlag(options::OPT_use_spirv_backend,
                   options::OPT_no_use_spirv_backend,
                   /*Default=*/true);
  InputInfo LinkedBCInput = InputInfo(types::TY_LLVM_BC, TempFile, "");

  if (UseSPIRVBackend) {
    llvm::opt::ArgStringList CmdArgs;
    const char *Triple = C.getArgs().MakeArgString("-triple=spirv64-amd-amdhsa");
    CmdArgs.append({"-cc1", Triple, "-emit-obj", "-disable-llvm-optzns",
                    LinkedBCInput.getFilename(), "-o", Output.getFilename()});

    const Driver &Driver = getToolChain().getDriver();
    const char *Exec = Driver.getClangProgramPath();
    C.addCommand(std::make_unique<Command>(
        JA, *this, ResponseFileSupport::None(), Exec, CmdArgs, LinkedBCInput,
        Output, Driver.getPrependArg()));
    return;
  }

  llvm::opt::ArgStringList TrArgs;
  TrArgs.push_back("--spirv-max-version=1.6");
  TrArgs.push_back("--spirv-ext=+all,-SPV_KHR_untyped_pointers");
  TrArgs.push_back("--spirv-allow-unknown-intrinsics");
  TrArgs.push_back("--spirv-lower-const-expr");
  TrArgs.push_back("--spirv-preserve-auxdata");
  TrArgs.push_back("--spirv-debug-info-version=nonsemantic-shader-200");
  SPIRV::constructTranslateCommand(C, *this, JA, Output, LinkedBCInput, TrArgs);
}

void Linker::ConstructJob(Compilation &C, const JobAction &JA,
                          const InputInfo &Output, const InputInfoList &Inputs,
                          const llvm::opt::ArgList &Args,
                          const char *LinkingOutput) const {
  constructLinkAndEmitSpirvCommand(C, JA, Inputs, Output, Args);
}

} // namespace clang::driver::tools::SPIRVOpenMP

namespace clang::driver::toolchains {

SPIRVOpenMPToolChain::SPIRVOpenMPToolChain(const Driver &D,
                                           const llvm::Triple &Triple,
                                           const ToolChain &HostToolchain,
                                           const ArgList &Args)
    : SPIRVToolChain(D, Triple, Args), HostTC(HostToolchain) {
  getProgramPaths().push_back(getDriver().Dir);
}

Tool *SPIRVOpenMPToolChain::buildLinker() const {
  return new tools::SPIRVOpenMP::Linker(*this);
}

void SPIRVOpenMPToolChain::addClangTargetOptions(
    const llvm::opt::ArgList &DriverArgs, llvm::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadingKind) const {
  HostTC.addClangTargetOptions(DriverArgs, CC1Args, DeviceOffloadingKind);

  if (DeviceOffloadingKind != Action::OFK_OpenMP)
    return;

  // For SPIR-V we want to retain the pristine output of Clang CodeGen, since
  // optimizations might lose structure / information that is necessary for
  // generating optimal concrete AMDGPU code at JIT time. The JIT compiler
  // (COMGR) will apply optimizations when translating SPIR-V to native ISA.
  if (!DriverArgs.hasArg(options::OPT_disable_llvm_passes))
    CC1Args.push_back("-disable-llvm-passes");

  // Keep this close to HIPSPV behavior while prototyping.
  CC1Args.append({"-mllvm", "-vectorize-loops=false", "-mllvm",
                  "-vectorize-slp=false"});

  if (!DriverArgs.hasArg(options::OPT_fvisibility_EQ,
                         options::OPT_fvisibility_ms_compat))
    CC1Args.append({"-fvisibility=hidden", "-fapply-global-visibility-to-externs"});

  if (!DriverArgs.hasFlag(options::OPT_offloadlib, options::OPT_no_offloadlib,
                          true))
    return;

  addOpenMPDeviceRTL(getDriver(), DriverArgs, CC1Args, "", getTriple(), HostTC);
}

void SPIRVOpenMPToolChain::addClangWarningOptions(ArgStringList &CC1Args) const {
  HostTC.addClangWarningOptions(CC1Args);
}

ToolChain::CXXStdlibType
SPIRVOpenMPToolChain::GetCXXStdlibType(const ArgList &Args) const {
  return HostTC.GetCXXStdlibType(Args);
}

void SPIRVOpenMPToolChain::AddClangSystemIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  HostTC.AddClangSystemIncludeArgs(DriverArgs, CC1Args);
}

void SPIRVOpenMPToolChain::AddClangCXXStdlibIncludeArgs(
    const ArgList &Args, ArgStringList &CC1Args) const {
  HostTC.AddClangCXXStdlibIncludeArgs(Args, CC1Args);
}

llvm::SmallVector<ToolChain::BitCodeLibraryInfo, 12>
SPIRVOpenMPToolChain::getDeviceLibs(const llvm::opt::ArgList &DriverArgs,
                                    Action::OffloadKind DeviceOffloadKind) const {
  llvm::SmallVector<BitCodeLibraryInfo, 12> BCLibs;

  if (!DriverArgs.hasFlag(options::OPT_offloadlib, options::OPT_no_offloadlib,
                          true))
    return {};

  std::string BCName = "libomptarget-spirv.bc";

  if (const Arg *A =
          DriverArgs.getLastArg(options::OPT_libomptarget_spirv_bc_path_EQ)) {
    SmallString<128> LibOmpTargetFile(A->getValue());
    if (llvm::sys::fs::exists(LibOmpTargetFile) &&
        llvm::sys::fs::is_directory(LibOmpTargetFile)) {
      llvm::sys::path::append(LibOmpTargetFile, BCName);
    }

    if (llvm::sys::fs::exists(LibOmpTargetFile)) {
      BCLibs.emplace_back(std::string(LibOmpTargetFile));
      return BCLibs;
    }

    getDriver().Diag(diag::err_drv_omp_offload_target_bcruntime_not_found)
        << LibOmpTargetFile;
    return BCLibs;
  }

  SmallVector<StringRef, 8> LibraryPaths;

  if (auto LibPath = llvm::sys::Process::GetEnv("LIBRARY_PATH")) {
    SmallVector<StringRef, 8> Frags;
    const char EnvPathSeparatorStr[] = {llvm::sys::EnvPathSeparator, '\0'};
    llvm::SplitString(*LibPath, Frags, EnvPathSeparatorStr);
    for (StringRef Path : Frags)
      LibraryPaths.emplace_back(Path.trim());
  }

  for (const auto &LibPath : HostTC.getFilePaths())
    LibraryPaths.emplace_back(LibPath);

  SmallString<128> TripleLibPath(getDriver().Dir);
  llvm::sys::path::append(TripleLibPath, "..", "lib", getTriple().getTriple());
  LibraryPaths.emplace_back(TripleLibPath);

  for (StringRef LibraryPath : LibraryPaths) {
    SmallString<128> LibOmpTargetFile(LibraryPath);
    llvm::sys::path::append(LibOmpTargetFile, BCName);
    if (llvm::sys::fs::exists(LibOmpTargetFile)) {
      BCLibs.emplace_back(std::string(LibOmpTargetFile));
      return BCLibs;
    }
  }

  getDriver().Diag(diag::err_drv_omp_offload_target_missingbcruntime)
      << BCName << "spirv";

  return BCLibs;
}

SanitizerMask SPIRVOpenMPToolChain::getSupportedSanitizers() const {
  return HostTC.getSupportedSanitizers();
}

VersionTuple
SPIRVOpenMPToolChain::computeMSVCVersion(const Driver *D,
                                         const ArgList &Args) const {
  return HostTC.computeMSVCVersion(D, Args);
}

void SPIRVOpenMPToolChain::adjustDebugInfoKind(
    llvm::codegenoptions::DebugInfoKind &DebugInfoKind,
    const llvm::opt::ArgList &Args) const {
  DebugInfoKind = llvm::codegenoptions::NoDebugInfo;
}

} // namespace clang::driver::toolchains
