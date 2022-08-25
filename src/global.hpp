#ifndef TASINPUT2_GLOBAL_HPP_INCLUDED
#define TASINPUT2_GLOBAL_HPP_INCLUDED

#include <mupen64plus/m64p_types.h>
#include <mupen64plus/m64p_plugin.h>

#define TASINPUT2_CHECK_INITED \
if (!::tasinput::IsInit()) \
  return M64ERR_NOT_INIT
  
#define TASINPUT2_CHECK_NOT_INITED \
if (::tasinput::IsInit()) \
  return M64ERR_ALREADY_INIT

namespace tasinput {
  void InitGlobals(
    m64p_dynlib_handle core_handle, void* context,
    void (*on_debug)(void*, int, const char*));
    
  void InitControls(CONTROL* ctrl_states);
  
  // Returns true if initialized.
  bool IsInit();
  
  m64p_dynlib_handle GetCoreHandle();
  
  // Log a debug message to the frontend.
  void DebugLog(m64p_msg_level level, const char* msg);
}  // namespace tasinput

#endif