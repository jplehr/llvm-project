include "llvm/Target/Target.td"

def MEDA26InstrInfo : InstrInfo;
def MEDA26 : Target {
  let InstructionSet = MEDA26InstrInfo;
}

def : ProcessorModel<"meda26v1", NoSchedModel,[]>;
