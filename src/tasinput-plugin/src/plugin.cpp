#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <syncstream>
#include <system_error>
#define M64P_PLUGIN_PROTOTYPES

#include <mupen64plus/m64p_common.h>
#include <mupen64plus/m64p_config.h>
#include <mupen64plus/m64p_plugin.h>
#include <mupen64plus/m64p_types.h>
#undef IMPORT
#undef EXPORT

#if defined(__linux__) || defined(__APPLE__)
  #include <dlfcn.h>
  #define EXEFILE(path) path
#elif defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
  #define EXEFILE(path) path ".exe"
#endif

#include <filesystem>
#include <optional>

#include "config.hpp"
#include "oslib/plibdl.hpp"

#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>

#include <tnp/ipc_layout.hpp>

#include <tnp_prtc.pb.h>

namespace fs = std::filesystem;
namespace bp = boost::process;

namespace {
  // Simple union that handles the delayed construction of an object. The object
  // must be constructed at some point to avoid invoking UB. Accessing the value
  // is UB before calling construct().
  template <class T>
  union delay_ctor {
  private:
    char _dummy_please_ignore;

  public:
    T v;

    delay_ctor() : _dummy_please_ignore(0) {}
    ~delay_ctor() { v.~T(); }

    operator T&() { return v; }
    
    T* operator->() {
      return std::addressof(v);
    }

    template <class... Args>
    void construct(Args... args) {
      new (&v) T(std::forward<Args>(args)...);
    }
  };

#define M64P_FN(name) ptr_##name name;
  M64P_FN(ConfigOpenSection)
  M64P_FN(ConfigGetParamString)
  M64P_FN(ConfigSetDefaultString)
  M64P_FN(ConfigSaveFile)
#undef M64P_FN
  void* debug_ctx;
  void (*debug_fn)(void* debug_ctx, int level, const char* str);
  CONTROL* ctrl_arr;

  fs::path tasinput_path;
  fs::path get_own_path();

  std::optional<bp::child> proc;
  std::optional<bp::opstream> proc_cin;
  std::optional<bp::ipstream> proc_cout;

  delay_ctor<tnp::shm_server> server;

  std::string query_proc(const std::string& input) {
    *proc_cin << input << std::endl;

    std::string res;
    std::getline(*proc_cout, res);
    return res;
  }
#if defined(_WIN32)
  HMODULE self_hmod;
  fs::path get_own_path() {
    wchar_t buffer[MAX_PATH] = {};
    if (!GetModuleFileNameW(self_hmod, buffer, sizeof(buffer))) {
      int err = GetLastError();
      throw std::system_error(err, std::system_category());
    }
    return buffer;
  }
#elif defined(__linux__) || defined(__APPLE__)
  fs::path get_own_path() {
    Dl_info info;
    dladdr(reinterpret_cast<void*>(&PluginStartup), &info);
    return info.dli_fname;
  }
#endif
}  // namespace

#if defined(_WIN32)
BOOL APIENTRY DllMain(HMODULE hmod, DWORD reason, LPVOID reserved) {
  self_hmod = hmod;
  return TRUE;
}
#endif

namespace tnp {
  void m64p_log(int level, const char* msg) { debug_fn(debug_ctx, level, msg); }
}  // namespace tnp

extern "C" {

m64p_error PluginStartup(
  m64p_dynlib_handle core_hnd, void* debug_ctx,
  void (*debug_fn)(void*, int, const char*)) {
  ::debug_ctx = debug_ctx;
  ::debug_fn  = debug_fn;
// Load config functions
#define M64P_LOAD(fn) fn = oslib::pdlsym<ptr_##fn>(core_hnd, #fn);
  M64P_LOAD(ConfigOpenSection)
  M64P_LOAD(ConfigGetParamString)
  M64P_LOAD(ConfigSetDefaultString)
  M64P_LOAD(ConfigSaveFile)
#undef M64P_LOAD

  m64p_handle sect_hnd;
  ConfigOpenSection("TASInput", &sect_hnd);

  // Check if there is already a config entry
  const char* bin_path = ConfigGetParamString(sect_hnd, "TASInputBinary");
  if (bin_path == nullptr || bin_path[0] == '\0') {
    // If there isn't a config entry, then try to autodetect
    auto expect = get_own_path().parent_path() / EXEFILE("tasinput-qt");
    if (!fs::exists(expect)) {
      tnp::m64p_log(M64MSG_ERROR, "Could not find " EXEFILE("tasinput-qt"));
      puts("After not finding exe");
      return M64ERR_FILES;
    }
    ConfigSetDefaultString(
      sect_hnd, "TASInputBinary",
      reinterpret_cast<const char*>(expect.u8string().c_str()),
      "Path to " EXEFILE("tasinput-qt") ".");
    ConfigSaveFile();
    tasinput_path = expect;
  }
  else {
    tasinput_path = reinterpret_cast<const char8_t*>(bin_path);
  }

  tnp::m64p_log(M64MSG_STATUS, "Loading TASInput binary");
  
  server.construct();
  
  proc_cin.emplace();
  proc_cout.emplace();
  proc.emplace(
    tasinput_path.c_str(), server.v.id(), bp::std_in<(*proc_cin), bp::std_out>(*proc_cout));
  
  tnp::prtc::Ping msg;
  server->ipc_data().push_request(&tnp::ipc_layout::mq_p2e, msg, "ping");
  
  return M64ERR_SUCCESS;
}

m64p_error PluginShutdown() {
  if (query_proc("quit") != "DONE\n") {
    return M64ERR_INTERNAL;
  }
  proc->wait();
  proc_cin.reset();
  proc_cout.reset();
  return M64ERR_SUCCESS;
}

m64p_error PluginGetVersion(
  m64p_plugin_type* type, int* version, int* api_version,
  const char** plugin_name, int* caps) {
  if (type)
    *type = M64PLUGIN_INPUT;
  if (version)
    *version = TNP_VERSION_HEX;
  if (api_version)
    *api_version = 0x020101;
  if (plugin_name)
    *plugin_name = TNP_DISPLAY_NAME;
  if (caps)
    *caps = 0;

  return M64ERR_SUCCESS;
}

int RomOpen() {
  if (std::string resp = query_proc("show"); resp.starts_with("ERR:")) {
    resp.erase(0, 4);
    tnp::m64p_log(M64MSG_ERROR, resp.c_str());
    return false;
  }
  return true;
}

void RomClosed() {
  if (std::string resp = query_proc("hide"); resp.starts_with("ERR:")) {
    resp.erase(0, 4);
    tnp::m64p_log(M64MSG_ERROR, resp.c_str());
  }
}

void InitiateControllers(CONTROL_INFO ControlInfo) {
  ctrl_arr = ControlInfo.Controls;

  ctrl_arr[0].Present = true;
  ctrl_arr[0].Plugin  = PLUGIN_NONE;
}

void GetKeys(int ctrl, BUTTONS* keys) {
  if (ctrl != 0) {
    keys->Value = 0;
    return;
  }
  std::string resp = query_proc("query");
  if (resp.starts_with("ERR:")) {
    resp.erase(0, 4);
    tnp::m64p_log(M64MSG_ERROR, resp.c_str());
    keys[ctrl].Value = 0;
    return;
  }
  keys->Value = std::stoul(resp, nullptr, 16);
}

void ControllerCommand(int Control, unsigned char* Command) {}
void ReadController(int Control, unsigned char* Command) {}

void SDL_KeyDown(int keymod, int keysym) {}
void SDL_KeyUp(int keymod, int keysym) {}
}