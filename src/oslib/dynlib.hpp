#ifndef OSLIB_DYNLIB_HPP_INCLUDED
#define OSLIB_DYNLIB_HPP_INCLUDED

#include "preproc.hpp"
/*
Implementation of a dlsym-like API for managing shared libraries.
*/

#if OSLIB_GCC_UNKNOWN_REGION_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#ifdef OSLIB_OS_WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef _WINSOCKAPI_
    #define _WINSOCKAPI_
  #endif
#endif

#include <mupen64plus/m64p_types.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

namespace oslib {
  inline m64p_dynlib_handle dlopen(const char* name);

  template <class T = void>
  requires(std::is_pointer_v<T>) inline T
    dlsym(m64p_dynlib_handle hnd, const char* name);

  inline void dlclose(m64p_dynlib_handle hnd);
}  // namespace oslib

#if defined(OSLIB_OS_POSIX)
#pragma region POSIX-compliant implementation
  #include <dlfcn.h>
namespace oslib {
  inline m64p_dynlib_handle dlopen(const char* name) {
    if (::dlopen(name, RTLD_NOLOAD)) {
      throw std::runtime_error("Library is already loaded");
    }

    void* res = ::dlopen(name, RTLD_LAZY);
    if (!res) {
      const std::string err = dlerror();
      throw std::runtime_error(err);
    }
    return res;
  }

  template <class T>
  requires(std::is_pointer_v<T>) inline T
    dlsym(m64p_dynlib_handle hnd, const char* name) {
    dlerror();
    void* sym       = ::dlsym(hnd, name);
    const char* err = dlerror();
    if (err) {
      throw std::runtime_error(err);
    }
    return reinterpret_cast<T>(sym);
  }

  inline void dlclose(m64p_dynlib_handle hnd) {
    ::dlclose(hnd);
  }
}  // namespace oslib
#pragma endregion
#elif defined(OSLIB_OS_WIN32)
#pragma region Windows impl
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef _WINSOCKAPI_
    #define _WINSOCKAPI_
  #endif
  #include <windows.h>
  #include "details/convert.hpp"
namespace oslib {

  inline m64p_dynlib_handle dlopen(const char* name) {
    auto wstr_name = details::to_utf16(name);

    HMODULE hnd = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, wstr_name.c_str(), &hnd)) {
      throw std::runtime_error("Library is already loaded");
    }

    hnd = LoadLibraryW(wstr_name.c_str());
    if (!hnd) {
      int err = GetLastError();
      throw std::system_error(err, std::system_category());
    }
    return hnd;
  }

  template <class T>
  requires(std::is_pointer_v<T>) inline T
    dlsym(m64p_dynlib_handle hnd, const char* name) {
    SetLastError(0);
    FARPROC fn = GetProcAddress(hnd, name);
    int err    = GetLastError();
    if (err != ERROR_SUCCESS) {
      throw std::system_error(err, std::system_category());
    }
    return reinterpret_cast<T>(fn);
  }

  inline void dlclose(m64p_dynlib_handle hnd) {
    FreeLibrary(hnd);
  }
}
#pragma endregion
#else
#error Unsupported platform!
#endif

#if OSLIB_GCC_UNKNOWN_REGION_PRAGMA
#pragma GCC diagnostic pop
#endif

#endif