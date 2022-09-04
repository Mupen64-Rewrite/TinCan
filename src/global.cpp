#include "global.hpp"

#include <wx/app.h>
#include <wx/event.h>
#include <wx/init.h>
#include <wx/thread.h>

#include "core_fns.hpp"
#include "gui/application.hpp"
#include "mupen64plus/m64p_plugin.h"
#include "mupen64plus/m64p_types.h"

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
    ctrl_states[0] = {
      .Present = true, .Plugin = PLUGIN_NONE, .Type = CONT_TYPE_STANDARD};
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

  static void Test() {}

  // GUI Thread Stuff
  // ============================
  void StartGuiThread() {
    std::lock_guard __lock__(gui_startup_mutex);

    // This is a "gating semaphore".
    // It starts at 0. This thread tries to acquire it,
    // but has to wait for another thread to release it first.
    std::binary_semaphore await_init_sem(0);
    gui_thread = std::thread([&]() mutable {
      wxDISABLE_DEBUG_SUPPORT();

      wxInitializer wx_init;
      if (!wx_init.IsOk()) {
        gui_status = -1;
        return;
      }

      await_init_sem.release();
      
      tasinput::DebugLog(M64MSG_INFO, "Calling wxEntry...");

      // since I have no real argc/argv to pass
      // I can only pass in these fake versions
      wxEntry(fake_argc, fake_argv);
    });
    await_init_sem.acquire();
    tasinput::DebugLog(M64MSG_INFO, "Queuing SHOW_WINDOW event...");
    wxQueueEvent(
      wxApp::GetInstance(),
      new wxThreadEvent(wxEVT_THREAD, tasinput::GUI_SHOW_WINDOW));
  }
  
  void ShowGui() {
    
  }

  void StopGuiThread() {
    std::lock_guard __lock__(gui_startup_mutex);

    wxQueueEvent(
      wxApp::GetInstance(),
      new wxThreadEvent(wxEVT_THREAD, tasinput::GUI_CLEANUP));
      
    tasinput::DebugLog(M64MSG_INFO, "Joined GUI thread");

    if (gui_thread.joinable())
      gui_thread.join();
      
    tasinput::DebugLog(M64MSG_INFO, "Joined GUI thread");
  }

}  // namespace tasinput