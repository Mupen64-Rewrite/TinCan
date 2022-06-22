#ifndef _PLIBDL_HPP_
#define _PLIBDL_HPP_
/*
Implementation of a dlsym-like API for managing shared libraries.
*/

#include <mupen64plus/m64p_types.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

#if defined(__linux__) || defined(__APPLE__)
  #include <dlfcn.h>
#elif defined(_WIN32)
  #include <windows.h>
#endif

namespace oslib {
  inline m64p_dynlib_handle pdlopen(const char* name);

  template <class T = void>
  requires(std::is_pointer_v<T>) inline T
    pdlsym(m64p_dynlib_handle hnd, const char* name);

  inline void pdlclose(m64p_dynlib_handle hnd);

#if defined(__linux__) || defined(__APPLE__)

  inline m64p_dynlib_handle pdlopen(const char* name) {
    if (dlopen(name, RTLD_NOLOAD)) {
      throw std::runtime_error("Library is already loaded");
    }

    void* res = dlopen(name, RTLD_LAZY);
    if (!res) {
      const std::string err = dlerror();
      throw std::runtime_error(err);
    }
    return res;
  }

  template <class T>
  requires(std::is_pointer_v<T>) inline T
    pdlsym(m64p_dynlib_handle hnd, const char* name) {
    dlerror();
    void* sym       = dlsym(hnd, name);
    const char* err = dlerror();
    if (err) {
      throw std::runtime_error(err);
    }
    return reinterpret_cast<T>(sym);
  }

  inline void pdlclose(m64p_dynlib_handle hnd) { dlclose(hnd); }

#elif defined(_WIN32)

  namespace detail {
    inline std::wstring to_utf16(const char* str) {
      int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, -1, NULL, 0);
      std::wstring res(len + 1, L'\0');
      MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, -1, res.data(), 0);
      res.pop_back();
      return res;
    }
  }  // namespace detail

  inline m64p_dynlib_handle pdlopen(const char* name) {
    auto wstr_name = detail::to_utf16(name);

    HMODULE hnd = nullptr;
    if (!GetModuleHandleExW(
          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, wstr_name.c_str(),
          &hnd)) {
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
    pdlsym(m64p_dynlib_handle hnd, const char* name) {
    SetLastError(0);
    FARPROC fn = GetProcAddress(hnd, name);
    int err    = GetLastError();
    if (err != ERROR_SUCCESS) {
      throw std::system_error(err, std::system_category());
    }
    return reinterpret_cast<T>(fn);
  }

  inline void pdlclose(m64p_dynlib_handle hnd) { FreeLibrary(hnd); }

#else
#endif
}  // namespace oslib

#endif