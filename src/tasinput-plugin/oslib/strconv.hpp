#ifndef _STRCONV_HPP_
#define _STRCONV_HPP_

#include <filesystem>
#include <string_view>

namespace oslib {
  using path_string = std::basic_string<std::filesystem::path::value_type>;
  inline path_string utf8_to_path(const char* str);
  inline path_string utf8_to_path(std::string_view str);
  
  #if defined(__linux__) || defined(__APPLE__)
  inline path_string utf8_to_path(const char* str) {
    return std::string(str);
  }
  inline path_string utf8_to_path(std::string_view str) {
    return std::string(str);
  }
  #elif defined(_WIN32)
  inline path_string utf8_to_path(const char* str) {
    int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, -1, NULL, 0);
    std::wstring res(len + 1, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, -1, res.data(), res.size());
    res.pop_back();
    return res;
  }
  inline path_string utf8_to_path(std::string_view str) {
    int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(), str.size(), NULL, 0);
    std::wstring res(len + 1, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(), str.size(), res.data(), res.size());
    res.pop_back();
    return res;
  }
  #endif
}
#endif