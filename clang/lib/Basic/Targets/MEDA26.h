#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {
class LLVM_LIBRARY_VISIBILITY MEDA26TargetInfo : public TargetInfo {
public:
  MEDA26TargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    resetDataLayout();
  }
  ~MEDA26TargetInfo() = default;

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override {}

  llvm::SmallVector<clang::Builtin::InfosShard>
  getTargetBuiltins() const override {
    return {};
  }
  clang::TargetInfo::BuiltinVaListKind getBuiltinVaListKind() const override {
    return {};
  }
  bool validateAsmConstraint(const char *&, ConstraintInfo &) const override {
    return true;
  }
  std::string_view getClobbers() const override { return ""; }
  llvm::ArrayRef<const char *> getGCCRegNames() const override { return {}; }
  llvm::ArrayRef<clang::TargetInfo::GCCRegAlias>
  getGCCRegAliases() const override {
    return {};
  }
};
} // namespace targets
} // namespace clang
