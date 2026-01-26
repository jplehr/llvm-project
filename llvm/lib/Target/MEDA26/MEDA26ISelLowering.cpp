#include "MEDA26ISelLowering.h"
#include "MEDA26Subtarget.h"

#include "llvm/Target/TargetMachine.h"

using namespace llvm;

llvm::MEDA26TargetLowering::MEDA26TargetLowering(const TargetMachine &TM,
                                                 const MEDA26Subtarget &STI)
    : TargetLowering(TM, STI) {}
