

#include "global.hpp"

#include "core_fns.hpp"
#include "ipc/shm_block.hpp"
#include "mupen64plus/m64p_plugin.h"
#include "oslib/mutex.hpp"
#include "oslib/shmem.hpp"
#include "oslib/process.hpp"
#include <mupen64plus/m64p_types.h>

#include <atomic>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <semaphore>
#include <string>
#include <thread>
#include <string_view>

namespace tasinput {
  namespace {
    bool init_flag = false;

    m64p_dynlib_handle core_handle = nullptr;

    void* debug_context                             = nullptr;
    void (*debug_callback)(void*, int, const char*) = nullptr;

    CONTROL* ctrl_states = nullptr;

    // GUI Thread Stuff
    // ========================
    std::optional<oslib::shm_object> shm_obj;
    std::optional<oslib::shm_mapping> shm_data;
    std::optional<oslib::process> gui_process;

    int fake_argc     = 1;
    char fake_argv0[] = "./tasinput2";
    char* fake_argv[] = {fake_argv0};
    
    ipc::shm_block& shm_block() {
      return shm_data.value().read<ipc::shm_block>(0);
    }
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
    
    // Create the SHM block
    shm_obj.emplace(sizeof(ipc::shm_block));
    shm_data.emplace(shm_obj->map());
    
    // Copy ctrl_states to the shared block
    // Since the GUI process hasn't started yet I don't have to lock the mutex
    std::copy(ctrl_states, ctrl_states + 4, shm_block().cstate.begin());
    
    // Start the GUI process and pray
    using namespace std::literals;
    gui_process.emplace("./tasinput2"sv, std::initializer_list<std::string_view> {
      std::to_string(uintptr_t(shm_obj->native_handle())), 
      std::to_string(uintptr_t(shm_obj->size()))
    });
  }
  
  void ShowUI() {
    shm_block().flags |= ipc::shm_block::shmflags::show;
  }
  
  void CloseUI() {
    
  }
  
  void Shutdown() {
    shm_block().flags |= ipc::shm_block::shmflags::stop;
    if (gui_process->joinable())
      gui_process->join();
    init_flag = false;
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