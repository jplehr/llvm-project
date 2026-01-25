#include "MEDA26TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TextAPI/Target.h"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMEDA26TargetInfo() {
  RegisterTarget<Triple::meda26, /*HsaJIT=*/false> X(
      getTheMEDA26Target(), /*Name=*/"meda26",
      /*Desc=*/"Example BE for DA Meetup 2026", /*BackendName*/ "MEDA26");
}

Target &llvm::getTheMEDA26Target() {
  static Target TheMEDA26Target;
  return TheMEDA26Target;
}
