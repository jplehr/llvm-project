#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"

#include "MEDA26Subtarget.h"

#pragma once

namespace llvm {

class MEDA26TargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  MEDA26TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);
  ~MEDA26TargetMachine() override;

  const MEDA26Subtarget *getSubtargetImpl(const Function &F) const;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

class MEDA26PassConfig : public TargetPassConfig {
public:
  MEDA26PassConfig(TargetMachine &TM, PassManagerBase &PM);
  void addIRPasses() override;
  bool addInstSelector() override;
};

} // namespace llvm
