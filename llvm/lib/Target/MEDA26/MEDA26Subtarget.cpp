#include "MEDA26Subtarget.h"

using namespace llvm;

llvm::MEDA26Subtarget::MEDA26Subtarget(const Triple &TT, StringRef CPU,
                                       StringRef FS, const TargetMachine &TM)
    : TargetSubtargetInfo(TT, CPU, /*TuneCPU=*/"", FS, /*PF=*/{}, /*PD=*/{},
                          /*WPR=*/{}, /*WL=*/nullptr, /*RA=*/nullptr,
                          /*IS=*/nullptr, /*OC=*/nullptr, /*FP=*/nullptr,
                          /*??=*/nullptr),
      TLInfo(TM, *this) {}

const TargetRegisterInfo *llvm::MEDA26Subtarget::getRegisterInfo() const {
  return {};
}
