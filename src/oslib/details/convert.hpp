#ifndef OSLIB_DETAILS_CONVERT_HPP_INCLUDED
#define OSLIB_DETAILS_CONVERT_HPP_INCLUDED

#include <system_error>

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCKAPI_
  #define _WINSOCKAPI_
#endif
#include <windows.h>
#include "../preproc.hpp"
#if defined(OSLIB_OS_WIN32)

namespace oslib::details {
  inline std::wstring to_utf16(const char* str) {
    int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, -1, NULL, 0);
    if (len == 0) {
      throw std::system_error(GetLastError(), std::system_category());
    }

    std::wstring res(len + 1, L'\0');

    int res_len =
      MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, -1, res.data(), res.size());
    if (res_len == 0) {
      throw std::system_error(GetLastError(), std::system_category());
    }

    res.pop_back();
    return res;
  }
  inline std::wstring to_utf16(std::string_view str) {
    int len = MultiByteToWideChar(
      CP_UTF8, MB_PRECOMPOSED, str.data(), str.size(), NULL, 0);
    if (len == 0) {
      throw std::system_error(GetLastError(), std::system_category());
    }

    std::wstring res(len, L'\0');

    int res_len = MultiByteToWideChar(
      CP_UTF8, MB_PRECOMPOSED, str.data(), str.size(), res.data(), res.size());
    if (res_len == 0) {
      throw std::system_error(GetLastError(), std::system_category());
    }

    return res;
  }
}  // namespace oslib::details

#endif
#endif