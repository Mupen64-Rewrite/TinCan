#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCKAPI_
  #define _WINSOCKAPI_
#endif

#include "core_fns.hpp"
#include "global.hpp"
#include "mupen64plus/m64p_config.h"
#include "oslib/dynlib.hpp"

#define TASINPUT_DEFINE_CORE_FN(                                              \
  return_t, name, params_with_types, params_without_types)                    \
  return_t name params_with_types {                                           \
    static ptr_##name fn = oslib::dlsym<ptr_##name>(GetCoreHandle(), #name); \
    return fn params_without_types;                                           \
  }

namespace tasinput::core {
  // I wish I could make a code generator for this stuff, but macros will have to do

  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigListSections,
    (void* context, void (*callback)(void*, const char*)), (context, callback))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigOpenSection, (const char* name, m64p_handle* section),
    (name, section))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigListParameters,
    (m64p_handle section, void* context,
     void (*callback)(void*, const char*, m64p_type)),
    (section, context, callback))
  TASINPUT_DEFINE_CORE_FN(m64p_error, ConfigSaveFile, (), ())
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSaveSection, (const char* name), (name))
  TASINPUT_DEFINE_CORE_FN(
    int, ConfigHasUnsavedChanges, (const char* name), (name))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigDeleteSection, (const char* name), (name))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigRevertChanges, (const char* name), (name))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSetParameter,
    (m64p_handle section, const char* name, m64p_type type,
     const void* p_value),
    (section, name, type, p_value))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSetParameterHelp,
    (m64p_handle section, const char* name, const char* help),
    (section, name, help))
  TASINPUT_DEFINE_CORE_FN(
    const char*, ConfigGetParameterHelp,
    (m64p_handle section, const char* name), (section, name))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigGetParameterType,
    (m64p_handle section, const char* name, m64p_type* type),
    (section, name, type))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSetDefaultInt,
    (m64p_handle section, const char* name, int value, const char* help),
    (section, name, value, help))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSetDefaultFloat,
    (m64p_handle section, const char* name, float value, const char* help),
    (section, name, value, help))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSetDefaultBool,
    (m64p_handle section, const char* name, int value, const char* help),
    (section, name, value, help))
  TASINPUT_DEFINE_CORE_FN(
    m64p_error, ConfigSetDefaultString,
    (m64p_handle section, const char* name, const char* value,
     const char* help),
    (section, name, value, help))
  TASINPUT_DEFINE_CORE_FN(
    int, ConfigGetParamInt, (m64p_handle section, const char* name),
    (section, name))
  TASINPUT_DEFINE_CORE_FN(
    float, ConfigGetParamFloat, (m64p_handle section, const char* name),
    (section, name))
  TASINPUT_DEFINE_CORE_FN(
    int, ConfigGetParamBool, (m64p_handle section, const char* name),
    (section, name))
  TASINPUT_DEFINE_CORE_FN(
    const char*, ConfigGetParamString, (m64p_handle section, const char* name),
    (section, name))
}  // namespace tasinput::core