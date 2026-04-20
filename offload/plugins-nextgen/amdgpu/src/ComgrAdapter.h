#ifndef OFFLOAD_PLUGINS_NEXTGEN_AMDGPU_COMGRADAPTER_H
#define OFFLOAD_PLUGINS_NEXTGEN_AMDGPU_COMGRADAPTER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"

#include <memory>
#include <string>

namespace llvm {
namespace omp {
namespace target {
namespace plugin {

enum class ComgrMode {
  Linked,
  Dynamic,
  Disabled,
};

/// Small adapter boundary around COMGR so AMDGPU plugin logic can stay mostly
/// COMGR-agnostic and we can support linked / dlopen / disabled builds.
class ComgrAdapter final {
public:
  ComgrAdapter() = default;

  static ComgrMode getMode();
  static const char *getModeName();

  /// Compile SPIR-V image bytes to an AMDGPU relocatable object.
  static Expected<std::unique_ptr<MemoryBuffer>>
  compileSPIRVToRelocatable(StringRef SPIRVImage, StringRef IsaName);

private:
  static Error createComgrError(const char *Prefix, int StatusCode);
};

} // namespace plugin
} // namespace target
} // namespace omp
} // namespace llvm

#endif // OFFLOAD_PLUGINS_NEXTGEN_AMDGPU_COMGRADAPTER_H
