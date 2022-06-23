#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <system_error>
#define M64P_PLUGIN_PROTOTYPES

#include <mupen64plus/m64p_common.h>
#include <mupen64plus/m64p_config.h>
#include <mupen64plus/m64p_plugin.h>
#include <mupen64plus/m64p_types.h>

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

#include "subp/subprocess.hpp"


namespace fs = std::filesystem;
namespace sp = subprocess;

namespace {
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
  
  std::optional<sp::Popen> proc;
  
  std::string query_proc(const std::string& input) {
    // working within the limitations of
    // the library. 4k should be enough for any query.
    thread_local std::array<char, 4096> ibuf {0};
    
    fputs((input + '\n').c_str(), proc->input());
    fflush(proc->input());
    fgets(ibuf.data(), ibuf.size(), proc->output());
    
    auto res = std::string(ibuf.data());
    res.pop_back();
    return res;
  }
  #ifdef _WIN32
  HMODULE self_hmod;
  fs::path get_own_path() { 
    wchar_t buffer[MAX_PATH] = {};
    if (!GetModuleFileNameW(self_hmod, buffer, sizeof(buffer))) {
      int err = GetLastError();
      throw std::system_error(err, std::system_category());
    }
    return buffer;
  }
  #endif
}  // namespace

#ifdef _WIN32
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

  // See if a config file entry exists
  m64p_handle sect_hnd;
  ConfigOpenSection("TASInput", &sect_hnd);

  const char* bin_path = ConfigGetParamString(sect_hnd, "TASInputBinary");
  if (bin_path == nullptr || bin_path[0] == '\0') {
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
  auto path_str = tasinput_path.string();
  proc.emplace({path_str.c_str()}, sp::output(sp::PIPE), sp::input(sp::PIPE));

  return M64ERR_SUCCESS;
}

m64p_error PluginShutdown() {
  if (query_proc("quit") != "DONE\n") {
    return M64ERR_INTERNAL;
  }
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
    keys[ctrl].Value = 0;
    return;
  }
  std::string resp = query_proc("query");
  if (resp.starts_with("ERR:")) {
    resp.erase(0, 4);
    tnp::m64p_log(M64MSG_ERROR, resp.c_str());
    keys[ctrl].Value = 0;
    return;
  }
  keys[ctrl].Value = std::stoul(resp, nullptr, 16);
}

void ControllerCommand(int Control, unsigned char* Command) {}
void ReadController(int Control, unsigned char* Command) {}

void SDL_KeyDown(int keymod, int keysym) {}
void SDL_KeyUp(int keymod, int keysym) {}
}

namespace {

#if defined(__linux__) || defined(__APPLE__)
  fs::path get_own_path() {
    Dl_info info;
    dladdr(reinterpret_cast<void*>(&PluginStartup), &info);
    return info.dli_fname;
  }
#endif
}  // namespace