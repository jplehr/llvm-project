#include "MEDA26ISelLowering.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/GlobalISel/InlineAsmLowering.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/RegisterBankInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/TargetParser/Triple.h"

#include "llvm/ADT/StringRef.h"

#pragma once

namespace llvm {

class MEDA26Subtarget : public TargetSubtargetInfo {
  virtual void anchor() {};
  MEDA26TargetLowering TLInfo;

public:
  MEDA26Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                  const TargetMachine &TM);
  ~MEDA26Subtarget() = default;

  const MEDA26TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const TargetRegisterInfo *getRegisterInfo() const override;
};
} // namespace llvm
