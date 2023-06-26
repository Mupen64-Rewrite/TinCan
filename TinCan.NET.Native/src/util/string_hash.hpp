#ifndef TC_UTIL_STRING_HASH_HPP
#define TC_UTIL_STRING_HASH_HPP

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
namespace tc {
  // basic string hasher for heterogeneous lookup.
  template <class T, class Traits = std::char_traits<T>>
  struct basic_string_hash {
    using hash_type = std::hash<std::basic_string_view<T, Traits>>;
    using is_transparent = void;

    size_t operator()(std::basic_string_view<T, Traits> sv) const {
      return hash_type {}(sv);
    }

    template <class A>
    size_t operator()(const std::basic_string<T, Traits, A>& str) const {
      return hash_type {}(str);
    }

    size_t operator()(const T* str) const { return hash_type {}(str); }
  };

  using string_hash    = basic_string_hash<char>;
  using wstring_hash   = basic_string_hash<wchar_t>;
  using u16string_hash = basic_string_hash<char16_t>;
  using u32string_hash = basic_string_hash<char32_t>;
#if __cpp_lib_char8_t >= 201907L
  using u8string_hash = basic_string_hash<char8_t>;
#endif

  template <class V>
  using string_map =
    std::unordered_map<std::string, V, string_hash, std::equal_to<>>;
  template <class V>
  using wstring_map =
    std::unordered_map<std::wstring, V, wstring_hash, std::equal_to<>>;
  template <class V>
  using u16string_map =
    std::unordered_map<std::u16string, V, u16string_hash, std::equal_to<>>;
  template <class V>
  using u32string_map =
    std::unordered_map<std::u32string, V, u32string_hash, std::equal_to<>>;
#if __cpp_lib_char8_t >= 201907L
  template <class V>
  using u8string_map =
    std::unordered_map<std::u8string, V, u8string_hash, std::equal_to<>>;
#endif

}  // namespace tc

#endif