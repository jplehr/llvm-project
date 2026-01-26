#include "MEDA26TargetTransformInfo.h"

using namespace llvm;

unsigned llvm::MEDA26TTIImpl::getLoadVectorFactor(unsigned VF,
                                                  unsigned LoadSize,
                                                  unsigned ChainSizeInBytes,
                                                  VectorType *VecTy) const {
  unsigned ElemSize = VecTy->getScalarSizeInBits();
  if (ElemSize < 32)
    return 8;

  return std::min(VF, 4u);
}
