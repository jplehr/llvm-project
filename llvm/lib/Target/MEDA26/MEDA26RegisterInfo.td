
let Namespace = "MEDA26" in {
def sub32_low: SubRegIndex<0,32>;
def sub32_high: SubRegIndex<32, 32>;

def sub64_low: SubRegIndex<0, 64>;
def sub64_high: SubRegIndex<64, 64>;

def s0 : Register<"s0">;
def s1 : Register<"s1">;
def s2 : Register<"s2">;
def s3 : Register<"s3">;

def d0 : Register<"d0"> {
  let SubRegIndices = [sub32_low, sub32_high];
  let SubRegs = [s0, s1];
  let CoveredBySubRegs = true;
}

def d1 : Register<"d1"> {
  let SubRegIndices = [sub32_low, sub32_high];
  let SubRegs = [s2, s3];
  let CoveredBySubRegs = true;
}

def q0 : Register<"q0"> {
  let SubRegIndices = [sub64_low, sub64_high];
  let SubRegs = [d0, d1];
  let CoveredBySubRegs = true;
}
}

def SINGLES : RegisterClass<"MEDA26", [i32, f32], 32, (add s0, s1, s2, s3)>;
def DOUBLES : RegisterClass<"MEDA26", [i64, f64], 64, (add d0, d1)>;
def QUADS : RegisterClass<"MEDA26", [f128], 128, (add q0)>;
