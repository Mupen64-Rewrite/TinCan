#include <bits/utility.h>
#include <QApplication>
#include <QMainWindow>
#include <QThread>
#include <array>
#include <atomic>
#include <bit>
#include <future>
#include <utility>
#include <limits>
#include <memory>
#include <string>
#include <concepts>

#include "main_window.hpp"

std::atomic_bool stopFlag = false;

template <std::unsigned_integral T>
constexpr T byteswap(T x) {
  static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
  
  constexpr size_t char_bit = std::numeric_limits<uint8_t>::digits;
  constexpr T mask = static_cast<T>(uint8_t(0) - uint8_t(1));
  constexpr size_t half_size = sizeof(T) / 2;
  
  T hi = []<size_t... Is>(std::index_sequence<Is...>, T x) {
    (((x & (mask << (Is + half_size))) >> ((2 * Is + 1) * char_bit)) | ...);
  }(std::make_index_sequence<half_size> {}, x);
  T lo = []<size_t... Is>(std::index_sequence<Is...>, T x) {
    (((x & (mask << (Is))) << ((2 * (half_size - 1 - Is) + 1) * char_bit)) | ...);
  }(std::make_index_sequence<half_size> {}, x);
  return hi | lo;
}
template <>
constexpr uint8_t byteswap(uint8_t x) {
  return x;
}
#ifdef __GNUC__
template <>
constexpr uint16_t byteswap(uint16_t x) {
  return __builtin_bswap16(x);
}
template <>
constexpr uint32_t byteswap(uint32_t x) {
  return __builtin_bswap32(x);
}
template <>
constexpr uint64_t byteswap(uint64_t x) {
  return __builtin_bswap64(x);
}
#endif

constexpr uint32_t test = byteswap<uint32_t>(0x01020304);

template <std::unsigned_integral T>
T network_swap(T x) {
  if constexpr (std::endian::native == std::endian::little) {
    return byteswap(x);
  }
  else if constexpr (std::endian::native == std::endian::big) {
    return x;
  }
  else {
    static_assert(sizeof(T) == 0, "Native byte encoding can't be determiend");
  }
}

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  tnp::MainWindow w;
  w.show();
  
  int res = a.exec();
}
