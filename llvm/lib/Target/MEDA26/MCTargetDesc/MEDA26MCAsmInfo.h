#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/TargetParser/Triple.h"

#pragma once

namespace llvm {

class MEDA26MCAsmInfoELF : public MCAsmInfoELF {
public:
  explicit MEDA26MCAsmInfoELF(const Triple &TT,
                              const MCTargetOptions &Options) {}
};

static MCAsmInfo *createMEDA26MCAsmInfo(const MCRegisterInfo &MRI,
                                        const Triple &TheTriple,
                                        const MCTargetOptions &Options) {
  MCAsmInfo *MAI;
  if (TheTriple.isOSBinFormatELF())
    MAI = new MEDA26MCAsmInfoELF(TheTriple, Options);
  else
    report_fatal_error("Binary format not supported");
  return MAI;
}

} // namespace llvm
