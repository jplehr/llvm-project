
#include "llvm/CodeGen/TargetFrameLowering.h"

#pragma once

namespace llvm {

class MEDA26FrameLowering : public TargetFrameLowering {
public:
  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

protected:
  bool hasFPImpl(const MachineFunction &MF) const override { return false; }
};
} // namespace llvm
