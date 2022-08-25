#ifndef TASINPUT2_CORE_FNS_HPP
#define TASINPUT2_CORE_FNS_HPP

#include <mupen64plus/m64p_config.h>
#include "mupen64plus/m64p_types.h"

#define TASINPUT2_CORE_FN(name, params) \
  ::tasinput::function_traits<ptr_##name>::type name params

namespace tasinput::core {
  // List sections in the config file. The callback is called once
  // for each section.
  m64p_error ConfigListSections(
    void* context, void (*callback)(void* context, const char* name));
    
  // Open a config section.
  m64p_error ConfigOpenSection(const char*, m64p_handle*);
  
  // List parameters in a config section. The callback is called once
  // for each parameter.
  m64p_error ConfigListParameters(
    m64p_handle section, void* context,
    void (*callback)(void* context, const char* name, m64p_type type));
    
  // Save ALL current config.
  m64p_error ConfigSaveFile();
  
  // Save a config section.
  m64p_error ConfigSaveSection(const char* name);
  
  // Check if there are unsaved changes in a section.
  int ConfigHasUnsavedChanges(const char* name);
  
  // Delete a config section.
  m64p_error ConfigDeleteSection(const char* name);
  
  // Revert any unsaved config changes in the named section.
  m64p_error ConfigRevertChanges(const char* name);
  
  // Set a parameter's value.
  m64p_error ConfigSetParameter(
    m64p_handle section, const char* name, m64p_type type, const void* p_value);
  
  // Set a parameter's help.
  m64p_error ConfigSetParameterHelp(
    m64p_handle section, const char* name, const char* help);
  
  // Get a parameter's help. If it doesn't have help or doesn't exist, returns nullptr.
  const char* ConfigGetParameterHelp(m64p_handle section, const char* name);
  
  // Get a parameter's current type.
  m64p_error ConfigGetParameterType(
    m64p_handle section, const char* name, m64p_type* type);
  
  // Set a parameter's value if it isn't already set, and set its help comment.
  m64p_error ConfigSetDefaultInt(
    m64p_handle section, const char* name, int value, const char* help);
  // Set a parameter's value if it isn't already set, and set its help comment.
  m64p_error ConfigSetDefaultFloat(
    m64p_handle section, const char* name, float value, const char* help);
  // Set a parameter's value if it isn't already set, and set its help comment.
  m64p_error ConfigSetDefaultBool(
    m64p_handle section, const char* name, int value, const char* help);
  // Set a parameter's value if it isn't already set, and set its help comment.
  m64p_error ConfigSetDefaultString(
    m64p_handle section, const char* name, const char* value, const char* help);
  
  // Unsafe: get a parameter's value. Errors are reported through the debug callback.
  int ConfigGetParamInt(m64p_handle section, const char* name);
  // Unsafe: get a parameter's value. Errors are reported through the debug callback.
  float ConfigGetParamFloat(m64p_handle section, const char* name);
  // Unsafe: get a parameter's value. Errors are reported through the debug callback.
  int ConfigGetParamBool(m64p_handle section, const char* name);
  // Unsafe: get a parameter's value. Errors are reported through the debug callback.
  const char* ConfigGetParamString(m64p_handle section, const char* name);

}  // namespace tasinput::core

#endif