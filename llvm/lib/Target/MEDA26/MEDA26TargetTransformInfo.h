#include "MEDA26Subtarget.h"
#include "MEDA26TargetMachine.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/IR/Function.h"

#pragma once
namespace llvm {
class MEDA26TTIImpl : public BasicTTIImplBase<MEDA26TTIImpl> {
  using BaseT = BasicTTIImplBase<MEDA26TTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const MEDA26Subtarget &ST;
  const MEDA26TargetLowering &TLI;

  const MEDA26Subtarget *getST() const { return &ST; }
  const MEDA26TargetLowering *getTLI() const { return &TLI; }

public:
  explicit MEDA26TTIImpl(const MEDA26TargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(*TM->getSubtargetImpl(F)),
        TLI(*ST.getTargetLowering()) {}
  virtual ~MEDA26TTIImpl() {}

  unsigned getLoadVectorFactor(unsigned VF, unsigned LoadSize,
                               unsigned ChainSizeInBytes,
                               VectorType *VecTy) const override;
};
} // namespace llvm
