
let Namespace = "MEDA26" in {
def MYINSTR: Instruction<> {
  let OutOperandList = (outs SINGLES:$dst0, SINGLES:$dst1);
  let InOperandList = (ins SINGLES:$src0, SINGLES:$src1);
}

def MYADD: Instruction<> {
  let OutOperandList = (outs SINGLES:$dst0);
  let InOperandList = (ins SINGLES:$src0, SINGLES:$src1);
}

def MYSUB: Instruction<> {
  let OutOperandList = (outs SINGLES:$dst0);
  let InOperandList = (ins SINGLES:$src0, SINGLES:$src1);
}

}
