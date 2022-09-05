#ifndef OSLIB_DETAILS_META_HPP_INCLUDED
#define OSLIB_DETAILS_META_HPP_INCLUDED

#include <version>
#include <utility>
#include <type_traits>
#include <cstddef>


#ifdef __cpp_lib_source_location
#include <source_location>
#include <functional>
#include <string_view>

#define OSLIB_META_DEFAULT_DISCRIMINATOR ::oslib::details::secret::hash_callsite()
namespace oslib::details::secret {
  template <class T0, class... TR>
  constexpr size_t combine_hashes(T0&& first, TR&&... rest) {
    size_t seed = std::hash<T0>()(first) + 0x9e3779b9;
    (void(seed ^= std::hash<TR>()(rest) + 0x9e3779b9 + (seed<<6) + (seed>>2)), ...);
    return seed;
  }
  
  consteval size_t hash_callsite(std::source_location srcloc = std::source_location::current()) {
    return combine_hashes(srcloc.line(), srcloc.column(), std::string_view(srcloc.file_name()), std::string_view(srcloc.function_name()));
  }
}
#define OSLIB_DETAILS_META_IS_COMPLETE(T) (::oslib::details::is_complete_v<T>)
#else
#define OSLIB_META_DEFAULT_DISCRIMINATOR 0xDEADBEEF
#define OSLIB_DETAILS_META_IS_COMPLETE(T) (::oslib::details::is_complete_v<T, __LINE__>)
#endif

namespace oslib::details {
  template <class T, size_t = OSLIB_META_DEFAULT_DISCRIMINATOR, class = void>
  struct is_complete : std::false_type {};
  
  template <class T, size_t D>
  struct is_complete<T, D, std::void_t<decltype(sizeof(T))>> : std::true_type {};
  
  template <class T, size_t D = OSLIB_META_DEFAULT_DISCRIMINATOR>
  inline constexpr bool is_complete_v = is_complete<T, D>::value;
}

#undef OSLIB_META_DEFAULT_DISCRIMINATOR
#endif