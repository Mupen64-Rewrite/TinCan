

#include "global.hpp"

#include <mupen64plus/m64p_types.h>
#include "core_fns.hpp"
#include "ipc/shm_block.hpp"
#include "mupen64plus/m64p_plugin.h"
#include "oslib/cfile.hpp"
#include "oslib/mutex.hpp"
#include "oslib/process.hpp"
#include "oslib/shmem.hpp"
#include "resdata.hpp"

#include <atomic>
#include <filesystem>
#include <initializer_list>
#include <mutex>
#include <optional>
#include <semaphore>
#include <string>
#include <string_view>
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
    std::optional<oslib::shm_object> shm_obj;
    std::optional<oslib::shm_mapping> shm_data;
    std::optional<oslib::process> gui_process;

    std::filesystem::path tmp_path;

    ipc::shm_block& shm_block() {
      return shm_data.value().read<ipc::shm_block>(0);
    }
    
    #if defined (OSLIB_OS_LINUX)
    inline constexpr std::string_view executable_extension = "";
    #elif defined(OSLIB_OS_WIN32)
    inline constexpr std::string_view executable_extension = ".exe";
    #endif
    
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
    for (int i = 0; i < 4; i++) {
      ctrl_states[i] = {
        .Present = false, .Plugin = PLUGIN_NONE, .Type = CONT_TYPE_STANDARD};
    }
    ctrl_states[0].Present = true;

    // Create the SHM block
    shm_obj.emplace(4096);
    shm_data.emplace(shm_obj->map());
    new (shm_data->addr<ipc::shm_block*>(0)) ipc::shm_block();

    // Copy ctrl_states to the shared block
    // Since the GUI process hasn't started yet I don't have to lock the mutex
    std::copy_n(ctrl_states, 4, shm_block().cstate.begin());

    // Start the GUI process and pray
    using namespace std::literals;

    auto [tmpfile, path] = oslib::create_tempfile(executable_extension);
    fwrite(_data_exedata, 1, _size_exedata, tmpfile);
    tmp_path = path;
    tmpfile.close();
    
    // In POSIX, chmod +x the file
    // This does nothing on Windows where file execution is arbitrary
#if defined(OSLIB_OS_POSIX)
    auto tmp_stat = std::filesystem::status(tmp_path);
    std::filesystem::perms plus_x = tmp_stat.permissions() | std::filesystem::perms::owner_exec;
    std::filesystem::permissions(tmp_path, plus_x);
    
    std::cerr << "Temporary path: " << tmp_path << '\n';
#endif
    
    gui_process.emplace(
      tmp_path.string(),
      std::initializer_list<std::string_view> {
        std::to_string(uintptr_t(shm_obj->native_handle())),
        std::to_string(uintptr_t(shm_obj->size()))});
  }

  void ShowUI() {
    shm_block().flags |= ipc::shm_block::shmflags::show;
  }

  void CloseUI() {
    shm_block().flags &= ~ipc::shm_block::shmflags::show;
  }

  BUTTONS GetInputs(int ctrl) {
    return BUTTONS {.Value = shm_block().inputs[ctrl]};
  }

  void Shutdown() {
    shm_block().flags |= ipc::shm_block::shmflags::stop;
    if (gui_process->joinable())
      gui_process->join();
    shm_block().~shm_block();
    
    std::filesystem::remove(tmp_path);
    
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