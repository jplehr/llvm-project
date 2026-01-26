#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"

#include "MEDA26Subtarget.h"

namespace llvm {

class MEDA26TargetMachine : public CodeGenTargetMachineImpl {
public:
  MEDA26TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);
  ~MEDA26TargetMachine() override;

  const MEDA26Subtarget *getSubtargetImpl(const Function &F) const;
};
} // namespace llvm
