#include "MEDA26TargetMachine.h"
#include "MEDA26Subtarget.h"
#include "MEDA26TargetObjectFile.h"
#include "MEDA26TargetTransformInfo.h"
#include "TargetInfo/MEDA26TargetInfo.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"

#include "llvm/MC/TargetRegistry.h"

#include <optional>

using namespace llvm;

static const char *MEDA26DataLayoutStr =
    "e-p:32:32:32-n16:32-i64:64-i32:32:32-i16:16:16-i1:8:8-f32:32:32-v32:32:32";

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMEDA26Target() {
  RegisterTargetMachine<MEDA26TargetMachine> X(getTheMEDA26Target());
}
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMEDA26TargetMachine() {}

static std::unique_ptr<MEDA26Subtarget> SubtargetSingleton = nullptr;

MEDA26TargetMachine::MEDA26TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, MEDA26DataLayoutStr, TT, CPU, FS, Options,
                               RM ? *RM : Reloc::Static,
                               CM ? *CM : CodeModel::Small, OL),
      TLOF(createTLOF(getTargetTriple())) {
  initAsmInfo();
}

MEDA26TargetMachine::~MEDA26TargetMachine() = default;

const MEDA26Subtarget *
MEDA26TargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  StringRef CPU = CPUAttr.isValid() ? CPUAttr.getValueAsString() : TargetCPU;
  StringRef FS = FSAttr.isValid() ? FSAttr.getValueAsString() : TargetFS;

  if (!SubtargetSingleton)
    SubtargetSingleton =
        std::make_unique<MEDA26Subtarget>(TargetTriple, CPU, FS, *this);
  return SubtargetSingleton.get();
}

TargetTransformInfo
MEDA26TargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<MEDA26TTIImpl>(this, F));
}

TargetPassConfig *MEDA26TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MEDA26PassConfig(*this, PM);
}

MEDA26PassConfig::MEDA26PassConfig(TargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

void MEDA26PassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();
  // FIXME this may need to be changed
}

bool MEDA26PassConfig::addInstSelector() { return false; }
