#include "MEDA26MCAsmInfo.h"
#include "TargetInfo/MEDA26TargetInfo.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

#define GET_SUBTARGETINFO_MC_DESC
#include "MEDA26GenSubtargetInfo.inc"

using namespace llvm;

static MCSubtargetInfo *
createMEDA26MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createMEDA26MCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCInstrInfo *createMEDA26MCInstrInfo() { return new MCInstrInfo(); }

static MCRegisterInfo *createMEDA26MCRegisterInfo(const Triple &Triple) {
  return new MCRegisterInfo();
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMEDA26TargetMC() {
  Target &TheTarget = getTheMEDA26Target();
  TargetRegistry::RegisterMCSubtargetInfo(TheTarget,
                                          createMEDA26MCSubtargetInfo);
  TargetRegistry::RegisterMCInstrInfo(TheTarget, createMEDA26MCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(TheTarget, createMEDA26MCRegisterInfo);
  RegisterMCAsmInfoFn X(TheTarget, createMEDA26MCAsmInfo);
}
