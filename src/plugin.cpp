#define M64P_PLUGIN_PROTOTYPES
#include <mupen64plus/m64p_common.h>
#include <mupen64plus/m64p_types.h>
#include <mupen64plus/m64p_plugin.h>

#include "config.hpp"

EXPORT m64p_error CALL PluginGetVersion(
  m64p_plugin_type* type, int* ver, int* api_ver, const char** name,
  int* caps) {
  
  if (type != nullptr)
    *type = M64PLUGIN_INPUT;
  
  if (ver != nullptr)
    *ver = TASINPUT_VERSION_HEX;
  
  if (api_ver != nullptr)
    *api_ver = TASINPUT_API_VERSION_HEX;
    
  if (name != nullptr)
    *name = TASINPUT_DISPLAY_NAME;
  
  if (caps != nullptr)
    *caps = 0;
    
  return M64ERR_SUCCESS;
}

// Start up the plugin.
EXPORT m64p_error CALL PluginStartup(
  m64p_dynlib_handle core_hnd, void* debug_ctx,
  void (*on_debug)(void*, int, const char*)) {
  
  
}

EXPORT m64p_error CALL PluginShutdown() {
  
  return M64ERR_SUCCESS;
}

EXPORT void CALL ControllerCommand(int idx, unsigned char* cmd) {
  
}
EXPORT void CALL GetKeys(int idx, BUTTONS* keys) {
  
}
EXPORT void CALL InitiateControllers(CONTROL_INFO info) {
  
}
EXPORT void CALL ReadController(int idx, unsigned char* cmd) {
  
}
EXPORT void CALL SDL_KeyDown(int keymod, int keysym) {
  
}
EXPORT void CALL SDL_KeyUp(int keymod, int keysym) {
  
}

// Used to allow the input plugin to draw some kind of HUD.
// We don't need it.
// EXPORT void CALL RenderCallback(void) {}

// VRU Functions (Not really useful unless someone TASes Hey You Pikachu)
// ======================================================================

// EXPORT void CALL SendVRUWord(uint16_t length, uint16_t* word, uint8_t lang) {}
// EXPORT void CALL SetMicState(int state) {}
// EXPORT void CALL ReadVRUResults(
//   uint16_t* error_flags, uint16_t* num_results, uint16_t* mic_level,
//   uint16_t* voice_level, uint16_t* voice_length, uint16_t* matches) {}
// EXPORT void CALL ClearVRUWords(uint8_t length) {}
// EXPORT void CALL SetVRUWordMask(uint8_t length, uint8_t* mask) {}