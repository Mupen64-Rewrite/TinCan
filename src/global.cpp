#include "global.hpp"
#include "mupen64plus/m64p_types.h"

namespace tasinput {
  namespace {
    bool init_flag = false;

    m64p_dynlib_handle core_handle = nullptr;

    void* debug_context                             = nullptr;
    void (*debug_callback)(void*, int, const char*) = nullptr;

    CONTROL* ctrl_states = nullptr;
  }  // namespace

  void InitGlobals(
    m64p_dynlib_handle core_handle, void* context,
    void (*on_debug)(void*, int, const char*)) {
    ::tasinput::core_handle = core_handle;
    
    ::tasinput::debug_context  = context;
    ::tasinput::debug_callback = on_debug;

    init_flag = true;
  }

  void InitControls(CONTROL* ctrl_states) {
    ::tasinput::ctrl_states = ctrl_states;
  }

  bool IsInit() {
    return init_flag;
  }

  m64p_dynlib_handle GetCoreHandle() {
    return core_handle;
  }

  void DebugLog(m64p_msg_level level, const char* msg) {
    if (debug_callback != nullptr)
      debug_callback(debug_context, level, msg);
  }
}  // namespace tasinput