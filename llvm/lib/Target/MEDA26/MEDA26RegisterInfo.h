
#define GET_REGINFO_ENUM
#include "MEDA26GenRegisterInfo.inc"

#pragma once

namespace llvm {

class MEDA26RegisterInfo : public MEDA26GenRegisterInfo {
public:
  virtual const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const {
    return nullptr;
  }

  virtual bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                                   unsigned FIOperandNum,
                                   RegScavenger *RS = nullptr) const {
    return false;
  }

  virtual Register getFrameRegister(const MachineFunction &MF) const {
    return {};
  }

  // virtual ArrayRef<const uint32_t *> getRegMasks() const = 0;

  // virtual ArrayRef<const char *> getRegMaskNames() const = 0;

  // virtual BitVector getReservedRegs(const MachineFunction &MF) const = 0;

  // virtual const RegClassWeight &getRegClassWeight(
  // const TargetRegisterClass *RC) const = 0;

  /// Get the weight in units of pressure for this register unit.
  // virtual unsigned getRegUnitWeight(MCRegUnit RegUnit) const = 0;

  /// Get the number of dimensions of register pressure.
  // virtual unsigned getNumRegPressureSets() const = 0;

  /// Get the name of this register unit pressure set.
  // virtual const char *getRegPressureSetName(unsigned Idx) const = 0;

  /// Get the register unit pressure limit for this dimension.
  /// This limit must be adjusted dynamically for reserved registers.
  // virtual unsigned getRegPressureSetLimit(const MachineFunction &MF,
  //  unsigned Idx) const = 0;

  /// Get the dimensions of register pressure impacted by this register class.
  /// Returns a -1 terminated array of pressure set IDs.
  // virtual const int *getRegClassPressureSets(
  // const TargetRegisterClass *RC) const = 0;

  /// Get the dimensions of register pressure impacted by this register unit.
  /// Returns a -1 terminated array of pressure set IDs.
  // virtual const int *getRegUnitPressureSets(MCRegUnit RegUnit) const = 0;
};
} // namespace llvm
