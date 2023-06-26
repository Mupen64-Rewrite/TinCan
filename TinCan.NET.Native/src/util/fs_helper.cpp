#include "fs_helper.hpp"
#include <filesystem>


#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

#include <dlfcn.h>
std::filesystem::path tc::get_own_path() {
  Dl_info dli;
  dladdr(reinterpret_cast<void*>(&tc::get_own_path), &dli);
  return std::filesystem::absolute(std::filesystem::path(dli.dli_fname));
}

#elif defined(_WIN32)
#include <windows.h>
std::filesystem::path tc::get_own_path() {
  HMODULE h_self;
  BOOL res1 = GetModuleHandleExW(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
      GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    reinterpret_cast<LPCWSTR>(&tc::get_own_path), &h_self);
  if (!res1) {
    throw std::system_error(GetLastError(), std::system_category());
  }

  std::wstring buffer(MAX_PATH, '\0');
  DWORD len = 0;
  while (true) {
    len = GetModuleFileNameW(h_self, buffer.data(), buffer.size());
    if (len == 0) {
      throw std::system_error(GetLastError(), std::system_category());
    }
    if (len < buffer.length()) {
      buffer.resize(len, '\0');
      break;
    }
    buffer.resize(buffer.size() * 2);
  }
  return buffer;
}

#endif