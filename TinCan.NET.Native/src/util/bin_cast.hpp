#ifndef TC_UTIL_BIN_CAST_HPP
#define TC_UTIL_BIN_CAST_HPP

namespace tc {
  template <class T>
  inline constexpr const char* bin_cast(const T& obj) {
    return (const char*) &obj;
  }
}

#endif