#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Instruction.h"

#pragma once

namespace llvm {

class MEDA26Subtarget;

class MEDA26TargetLowering : public TargetLowering {
public:
  explicit MEDA26TargetLowering(const TargetMachine &TM,
                                const MEDA26Subtarget &STI);
};
} // namespace llvm
