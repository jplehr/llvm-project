include "llvm/Target/Target.td"

include "MEDA26RegisterInfo.td"
include "MEDA26Instructions.td"

defm : RemapAllTargetPseudoPointerOperands<DOUBLES>;

def MEDA26InstrInfo : InstrInfo;

def MEDA26 : Target {
  let InstructionSet = MEDA26InstrInfo;
}

def : ProcessorModel<"meda26v1", NoSchedModel,[]>;
