#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetLoweringObjectFile.h"

namespace llvm {

class MEDA26_ELFTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  MEDA26_ELFTargetObjectFile() {};
};

static std::unique_ptr<TargetLoweringObjectFile> createTLOF(const Triple &TT) {
  if (TT.isOSBinFormatELF())
    return std::make_unique<MEDA26_ELFTargetObjectFile>();

  return nullptr;
}
} // namespace llvm
