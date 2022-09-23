

#include "global.hpp"

#include "core_fns.hpp"
#include "mupen64plus/m64p_plugin.h"
#include <mupen64plus/m64p_types.h>

#include <atomic>
#include <mutex>
#include <semaphore>
#include <thread>

namespace tasinput {
  namespace {
    bool init_flag = false;

    m64p_dynlib_handle core_handle = nullptr;

    void* debug_context                             = nullptr;
    void (*debug_callback)(void*, int, const char*) = nullptr;

    CONTROL* ctrl_states = nullptr;

    // GUI Thread Stuff
    // ========================

    std::mutex gui_startup_mutex;
    std::thread gui_thread          = {};
    std::atomic_uint32_t gui_status = 0;

    int fake_argc     = 1;
    char fake_argv0[] = "./tasinput2";
    char* fake_argv[] = {fake_argv0};

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

    // this is hardcoded until I add multiplayer
    // (or VRU, which is far less likely)
    for (int i = 0; i < 4; i++){
      ctrl_states[i] = {
        .Present = false, .Plugin = PLUGIN_NONE, .Type = CONT_TYPE_STANDARD};
    }
    ctrl_states[0].Present = true;
  }

  bool IsInit() {
    return init_flag;
  }

  m64p_dynlib_handle GetCoreHandle() {
    return core_handle;
  }

  CONTROL& ControlInfo(int idx) {
    return ctrl_states[idx];
  }

  void DebugLog(m64p_msg_level level, const char* msg) {
    if (debug_callback != nullptr)
      debug_callback(debug_context, level, msg);
  }

}  // namespace tasinput