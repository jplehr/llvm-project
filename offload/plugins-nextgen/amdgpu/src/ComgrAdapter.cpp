#include "ComgrAdapter.h"

#include "Shared/Debug.h"

#include "llvm/Support/DynamicLibrary.h"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <type_traits>

using namespace llvm::offload::debug;

namespace {

// Keep a local minimal COMGR surface so this plugin can be built even when
// external COMGR headers are not present.
enum amd_comgr_status_t : uint32_t {
  AMD_COMGR_STATUS_SUCCESS = 0x0,
};

enum amd_comgr_language_t : uint32_t {
  AMD_COMGR_LANGUAGE_HIP = 0x3,
};

enum amd_comgr_data_kind_t : uint32_t {
  AMD_COMGR_DATA_KIND_RELOCATABLE = 0x7,
  AMD_COMGR_DATA_KIND_SPIRV = 0x15,
};

enum amd_comgr_action_kind_t : uint32_t {
  AMD_COMGR_ACTION_COMPILE_SPIRV_TO_RELOCATABLE = 0x10,
};

struct amd_comgr_data_t {
  uint64_t handle;
};

struct amd_comgr_data_set_t {
  uint64_t handle;
};

struct amd_comgr_action_info_t {
  uint64_t handle;
};

using FnStatusString = amd_comgr_status_t (*)(amd_comgr_status_t, const char **);
using FnCreateData = amd_comgr_status_t (*)(amd_comgr_data_kind_t,
                                            amd_comgr_data_t *);
using FnReleaseData = amd_comgr_status_t (*)(amd_comgr_data_t);
using FnSetData = amd_comgr_status_t (*)(amd_comgr_data_t, size_t, const void *);
using FnSetDataName = amd_comgr_status_t (*)(amd_comgr_data_t, const char *);
using FnGetData = amd_comgr_status_t (*)(amd_comgr_data_t, size_t *, void *);
using FnCreateDataSet = amd_comgr_status_t (*)(amd_comgr_data_set_t *);
using FnDestroyDataSet = amd_comgr_status_t (*)(amd_comgr_data_set_t);
using FnDataSetAdd = amd_comgr_status_t (*)(amd_comgr_data_set_t, amd_comgr_data_t);
using FnCreateActionInfo = amd_comgr_status_t (*)(amd_comgr_action_info_t *);
using FnDestroyActionInfo = amd_comgr_status_t (*)(amd_comgr_action_info_t);
using FnActionInfoSetLanguage =
    amd_comgr_status_t (*)(amd_comgr_action_info_t, amd_comgr_language_t);
using FnActionInfoSetIsaName =
    amd_comgr_status_t (*)(amd_comgr_action_info_t, const char *);
using FnDoAction = amd_comgr_status_t (*)(amd_comgr_action_kind_t,
                                          amd_comgr_action_info_t,
                                          amd_comgr_data_set_t,
                                          amd_comgr_data_set_t);
using FnActionDataCount =
    amd_comgr_status_t (*)(amd_comgr_data_set_t, amd_comgr_data_kind_t, size_t *);
using FnActionDataGetData = amd_comgr_status_t (*)(
    amd_comgr_data_set_t, amd_comgr_data_kind_t, size_t, amd_comgr_data_t *);

struct ComgrApiTable {
  FnStatusString status_string = nullptr;
  FnCreateData create_data = nullptr;
  FnReleaseData release_data = nullptr;
  FnSetData set_data = nullptr;
  FnSetDataName set_data_name = nullptr;
  FnGetData get_data = nullptr;
  FnCreateDataSet create_data_set = nullptr;
  FnDestroyDataSet destroy_data_set = nullptr;
  FnDataSetAdd data_set_add = nullptr;
  FnCreateActionInfo create_action_info = nullptr;
  FnDestroyActionInfo destroy_action_info = nullptr;
  FnActionInfoSetLanguage action_info_set_language = nullptr;
  FnActionInfoSetIsaName action_info_set_isa_name = nullptr;
  FnDoAction do_action = nullptr;
  FnActionDataCount action_data_count = nullptr;
  FnActionDataGetData action_data_get_data = nullptr;

  bool isUsable() const {
    return status_string && create_data && release_data && set_data &&
           set_data_name && get_data && create_data_set && destroy_data_set &&
           data_set_add && create_action_info && destroy_action_info &&
           action_info_set_language && action_info_set_isa_name && do_action &&
           action_data_count && action_data_get_data;
  }
};

#if defined(LIBOMPTARGET_AMDGPU_COMGR_LINKED)
extern "C" {
amd_comgr_status_t amd_comgr_status_string(amd_comgr_status_t status,
                                           const char **status_string);
amd_comgr_status_t amd_comgr_create_data(amd_comgr_data_kind_t, amd_comgr_data_t *);
amd_comgr_status_t amd_comgr_release_data(amd_comgr_data_t);
amd_comgr_status_t amd_comgr_set_data(amd_comgr_data_t, size_t, const void *);
amd_comgr_status_t amd_comgr_set_data_name(amd_comgr_data_t, const char *);
amd_comgr_status_t amd_comgr_get_data(amd_comgr_data_t, size_t *, void *);
amd_comgr_status_t amd_comgr_create_data_set(amd_comgr_data_set_t *);
amd_comgr_status_t amd_comgr_destroy_data_set(amd_comgr_data_set_t);
amd_comgr_status_t amd_comgr_data_set_add(amd_comgr_data_set_t, amd_comgr_data_t);
amd_comgr_status_t amd_comgr_create_action_info(amd_comgr_action_info_t *);
amd_comgr_status_t amd_comgr_destroy_action_info(amd_comgr_action_info_t);
amd_comgr_status_t amd_comgr_action_info_set_language(amd_comgr_action_info_t,
                                                      amd_comgr_language_t);
amd_comgr_status_t amd_comgr_action_info_set_isa_name(amd_comgr_action_info_t,
                                                      const char *);
amd_comgr_status_t amd_comgr_do_action(amd_comgr_action_kind_t,
                                       amd_comgr_action_info_t,
                                       amd_comgr_data_set_t,
                                       amd_comgr_data_set_t);
amd_comgr_status_t amd_comgr_action_data_count(amd_comgr_data_set_t,
                                               amd_comgr_data_kind_t, size_t *);
amd_comgr_status_t amd_comgr_action_data_get_data(
    amd_comgr_data_set_t, amd_comgr_data_kind_t, size_t, amd_comgr_data_t *);
}

const ComgrApiTable &getLinkedApi() {
  static ComgrApiTable API = {
      amd_comgr_status_string,
      amd_comgr_create_data,
      amd_comgr_release_data,
      amd_comgr_set_data,
      amd_comgr_set_data_name,
      amd_comgr_get_data,
      amd_comgr_create_data_set,
      amd_comgr_destroy_data_set,
      amd_comgr_data_set_add,
      amd_comgr_create_action_info,
      amd_comgr_destroy_action_info,
      amd_comgr_action_info_set_language,
      amd_comgr_action_info_set_isa_name,
      amd_comgr_do_action,
      amd_comgr_action_data_count,
      amd_comgr_action_data_get_data};
  return API;
}
#endif

#if defined(LIBOMPTARGET_AMDGPU_COMGR_DYNAMIC)
const ComgrApiTable *getDynamicApi(std::string &ErrorMessage) {
  static ComgrApiTable API;
  static std::once_flag LoadOnce;
  static bool IsLoaded = false;
  static std::string LoadError;

  std::call_once(LoadOnce, [&]() {
    const char *Candidates[] = {"libamd_comgr.so", "libamd_comgr.so.2",
                                "libamd_comgr.so.1"};
    std::string LastError;
    std::unique_ptr<llvm::sys::DynamicLibrary> DynlibHandle;

    for (const char *Name : Candidates) {
      LastError.clear();
      auto Handle = std::make_unique<llvm::sys::DynamicLibrary>(
          llvm::sys::DynamicLibrary::getPermanentLibrary(Name, &LastError));
      if (Handle->isValid()) {
        DynlibHandle = std::move(Handle);
        ODBG(OLDT_Init) << "Loaded COMGR runtime from '" << Name << "'";
        break;
      }
    }

    if (!DynlibHandle) {
      LoadError = LastError.empty() ? "unable to load libamd_comgr.so"
                                    : LastError;
      return;
    }

    auto Resolve = [&](auto &Fn, const char *Name) -> bool {
      void *Sym = DynlibHandle->getAddressOfSymbol(Name);
      if (!Sym) {
        LoadError = std::string("missing COMGR symbol: ") + Name;
        return false;
      }
      Fn = reinterpret_cast<std::decay_t<decltype(Fn)>>(Sym);
      return true;
    };

    if (!Resolve(API.status_string, "amd_comgr_status_string") ||
        !Resolve(API.create_data, "amd_comgr_create_data") ||
        !Resolve(API.release_data, "amd_comgr_release_data") ||
        !Resolve(API.set_data, "amd_comgr_set_data") ||
        !Resolve(API.set_data_name, "amd_comgr_set_data_name") ||
        !Resolve(API.get_data, "amd_comgr_get_data") ||
        !Resolve(API.create_data_set, "amd_comgr_create_data_set") ||
        !Resolve(API.destroy_data_set, "amd_comgr_destroy_data_set") ||
        !Resolve(API.data_set_add, "amd_comgr_data_set_add") ||
        !Resolve(API.create_action_info, "amd_comgr_create_action_info") ||
        !Resolve(API.destroy_action_info, "amd_comgr_destroy_action_info") ||
        !Resolve(API.action_info_set_language,
                 "amd_comgr_action_info_set_language") ||
        !Resolve(API.action_info_set_isa_name,
                 "amd_comgr_action_info_set_isa_name") ||
        !Resolve(API.do_action, "amd_comgr_do_action") ||
        !Resolve(API.action_data_count, "amd_comgr_action_data_count") ||
        !Resolve(API.action_data_get_data, "amd_comgr_action_data_get_data")) {
      return;
    }

    IsLoaded = API.isUsable();
    if (!IsLoaded && LoadError.empty())
      LoadError = "COMGR API table is incomplete";
  });

  if (!IsLoaded) {
    ErrorMessage = LoadError;
    return nullptr;
  }

  return &API;
}
#endif

} // namespace

namespace llvm {
namespace omp {
namespace target {
namespace plugin {

ComgrMode ComgrAdapter::getMode() {
#if defined(LIBOMPTARGET_AMDGPU_COMGR_LINKED)
  return ComgrMode::Linked;
#elif defined(LIBOMPTARGET_AMDGPU_COMGR_DYNAMIC)
  return ComgrMode::Dynamic;
#else
  return ComgrMode::Disabled;
#endif
}

const char *ComgrAdapter::getModeName() {
  switch (getMode()) {
  case ComgrMode::Linked:
    return "linked";
  case ComgrMode::Dynamic:
    return "dynamic";
  case ComgrMode::Disabled:
    return "disabled";
  }
  return "unknown";
}

Error ComgrAdapter::createComgrError(const char *Prefix, int StatusCode) {
  return createStringError(inconvertibleErrorCode(), "%s (comgr status=%d)",
                           Prefix, StatusCode);
}

Expected<std::unique_ptr<MemoryBuffer>>
ComgrAdapter::compileSPIRVToRelocatable(StringRef SPIRVImage, StringRef IsaName) {
#if defined(LIBOMPTARGET_AMDGPU_COMGR_DISABLED) ||                             \
    (!defined(LIBOMPTARGET_AMDGPU_COMGR_LINKED) &&                            \
     !defined(LIBOMPTARGET_AMDGPU_COMGR_DYNAMIC))
  return createStringError(inconvertibleErrorCode(),
                           "SPIR-V runtime JIT unavailable: COMGR mode is "
                           "'%s'",
                           getModeName());
#else
  const ComgrApiTable *API = nullptr;
  std::string DynamicError;

#if defined(LIBOMPTARGET_AMDGPU_COMGR_LINKED)
  API = &getLinkedApi();
#elif defined(LIBOMPTARGET_AMDGPU_COMGR_DYNAMIC)
  API = getDynamicApi(DynamicError);
  if (!API)
    return createStringError(inconvertibleErrorCode(),
                             "SPIR-V runtime JIT unavailable in COMGR dynamic "
                             "mode: %s",
                             DynamicError.c_str());
#endif

  auto GetStatusString = [&](amd_comgr_status_t Status) -> const char * {
    const char *Message = nullptr;
    if (API && API->status_string &&
        API->status_string(Status, &Message) == AMD_COMGR_STATUS_SUCCESS &&
        Message)
      return Message;
    return "unknown COMGR error";
  };

  auto MakeStageError = [&](const char *Stage, amd_comgr_status_t Status)
      -> Error {
    return createStringError(inconvertibleErrorCode(),
                             "COMGR %s failed in mode '%s' for ISA '%s': %s "
                             "(status=%u)",
                             Stage, getModeName(), IsaName.str().c_str(),
                             GetStatusString(Status), unsigned(Status));
  };

  struct ComgrCleanup {
    const ComgrApiTable *API = nullptr;
    amd_comgr_data_t SPIRV = {0};
    amd_comgr_data_t Reloc = {0};
    amd_comgr_data_set_t SPIRVSet = {0};
    amd_comgr_data_set_t RelocSet = {0};
    amd_comgr_action_info_t Action = {0};

    ~ComgrCleanup() {
      if (!API)
        return;
      if (Reloc.handle)
        (void)API->release_data(Reloc);
      if (SPIRV.handle)
        (void)API->release_data(SPIRV);
      if (RelocSet.handle)
        (void)API->destroy_data_set(RelocSet);
      if (SPIRVSet.handle)
        (void)API->destroy_data_set(SPIRVSet);
      if (Action.handle)
        (void)API->destroy_action_info(Action);
    }
  } Cleanup;

  Cleanup.API = API;

  auto Check = [&](amd_comgr_status_t Status, const char *Stage) -> Error {
    if (Status == AMD_COMGR_STATUS_SUCCESS)
      return Error::success();
    return MakeStageError(Stage, Status);
  };

  if (auto Err = Check(API->create_data(AMD_COMGR_DATA_KIND_SPIRV,
                                        &Cleanup.SPIRV),
                       "create_data(SPIRV)"))
    return std::move(Err);
  if (auto Err = Check(API->set_data(Cleanup.SPIRV, SPIRVImage.size(),
                                     SPIRVImage.bytes_begin()),
                       "set_data(SPIRV)"))
    return std::move(Err);
  if (auto Err =
          Check(API->set_data_name(Cleanup.SPIRV, "runtime-input.spv"),
                "set_data_name(SPIRV)"))
    return std::move(Err);
  if (auto Err = Check(API->create_data_set(&Cleanup.SPIRVSet),
                       "create_data_set(SPIRV)"))
    return std::move(Err);
  if (auto Err = Check(API->data_set_add(Cleanup.SPIRVSet, Cleanup.SPIRV),
                       "data_set_add(SPIRV)"))
    return std::move(Err);

  if (auto Err = Check(API->create_action_info(&Cleanup.Action),
                       "create_action_info"))
    return std::move(Err);
  if (auto Err = Check(API->action_info_set_language(Cleanup.Action,
                                                     AMD_COMGR_LANGUAGE_HIP),
                       "action_info_set_language"))
    return std::move(Err);
  std::string IsaNameStr = IsaName.str();
  if (auto Err = Check(API->action_info_set_isa_name(Cleanup.Action,
                                                     IsaNameStr.c_str()),
                       "action_info_set_isa_name"))
    return std::move(Err);

  if (auto Err = Check(API->create_data_set(&Cleanup.RelocSet),
                       "create_data_set(RELOC)"))
    return std::move(Err);
  if (auto Err =
          Check(API->do_action(AMD_COMGR_ACTION_COMPILE_SPIRV_TO_RELOCATABLE,
                               Cleanup.Action, Cleanup.SPIRVSet, Cleanup.RelocSet),
                "do_action(COMPILE_SPIRV_TO_RELOCATABLE)"))
    return std::move(Err);

  size_t Count = 0;
  if (auto Err = Check(API->action_data_count(Cleanup.RelocSet,
                                              AMD_COMGR_DATA_KIND_RELOCATABLE,
                                              &Count),
                       "action_data_count(RELOC)"))
    return std::move(Err);
  if (Count == 0)
    return createStringError(inconvertibleErrorCode(),
                             "COMGR returned zero relocatable outputs for ISA "
                             "'%s'",
                             IsaNameStr.c_str());
  if (auto Err = Check(API->action_data_get_data(
                           Cleanup.RelocSet, AMD_COMGR_DATA_KIND_RELOCATABLE, 0,
                           &Cleanup.Reloc),
                       "action_data_get_data(RELOC)"))
    return std::move(Err);

  size_t OutputSize = 0;
  if (auto Err = Check(API->get_data(Cleanup.Reloc, &OutputSize, nullptr),
                       "get_data_size(RELOC)"))
    return std::move(Err);

  std::string Output;
  Output.resize(OutputSize);
  if (auto Err =
          Check(API->get_data(Cleanup.Reloc, &OutputSize, Output.data()),
                "get_data(RELOC)"))
    return std::move(Err);

  ODBG(OLDT_Init) << "COMGR mode '" << getModeName()
                  << "' compiled SPIR-V image to relocatable ("
                  << OutputSize << " bytes) for ISA " << IsaName;

  return MemoryBuffer::getMemBufferCopy(Output, "amdgpu-comgr-reloc");
#endif
}

} // namespace plugin
} // namespace target
} // namespace omp
} // namespace llvm
