#define M64P_PLUGIN_PROTOTYPES

#include <mupen64plus/m64p_types.h>
#include <mupen64plus/m64p_common.h>
#include <mupen64plus/m64p_plugin.h>

#include "cpp-subprocess/subprocess.hpp"

namespace {
  void* debug_ctx;
  void (* debug_fn)(void* debug_ctx, int level, const char* str);
}

extern "C" {

m64p_error PluginStartup(m64p_dynlib_handle core_hnd, void* debug_ctx, void (* debug_fn)(void*, int, const char*)) {
  ::debug_ctx = debug_ctx;
  ::debug_fn = debug_fn;
  
  return M64ERR_SUCCESS;
}

}