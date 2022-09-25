#ifndef OSLIB_CFILE_HPP_INCLUDED
#define OSLIB_CFILE_HPP_INCLUDED

#include <array>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <random>
#include <span>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include "secrand.hpp"

namespace oslib {
  // A wrapper around the C FILE*.
  class c_file {
  public:
    // Wrapper for fopen.
    c_file(const char* path, const char* open_mode) :
      m_file(fopen(path, open_mode)) {
      if (m_file == nullptr) {
        throw std::system_error(errno, std::generic_category());
      }
    }

    // Takes ownership over an existing FILE*.
    c_file(FILE* file_ptr) : m_file(file_ptr) {
      if (file_ptr == nullptr)
        throw std::invalid_argument("Provided FILE* was null");
    }

    c_file(const c_file&)            = delete;
    c_file& operator=(const c_file&) = delete;

    c_file(c_file&& rhs) : m_file(rhs.m_file) { rhs.m_file = nullptr; }
    c_file& operator=(c_file&& rhs) {
      std::swap(m_file, rhs.m_file);
      return *this;
    }

    // Wraps fclose.
    ~c_file() {
      if (m_file != nullptr)
        fclose(m_file);
    }

    void flush() {
      if (fflush(m_file) == EOF) {
        throw std::system_error(errno, std::generic_category());
      }
    }

    size_t read(char* buffer, size_t size) {
      errno      = 0;
      size_t res = fread(buffer, 1, size, m_file);
      if (errno != 0) {
        throw std::system_error(errno, std::generic_category());
      }
      return res;
    }

    size_t write(const char* buffer, size_t size) {
      errno      = 0;
      size_t res = fwrite(buffer, 1, size, m_file);
      if (errno != 0) {
        throw std::system_error(errno, std::generic_category());
      }
      return res;
    }

  private:
    FILE* m_file;
  };
  
  template <size_t N>
  inline void temp_chars(std::span<char, N> out) {
#if defined(OSLIB_OS_POSIX)
    static constexpr std::array<char, 62> rand_lut = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
#elif defined(OSLIB_OS_WIN32)
    static constexpr std::array<char, 36> rand_lut = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
#endif
    static oslib::secure_random_device srd {};
    static std::uniform_int_distribution<uint32_t> dist(0, rand_lut.size());
    
    for (char& c : out) {
      c = rand_lut[dist(srd)];
    }
  }

  inline c_file create_tempfile(std::string_view extension) {
    using namespace std::string_literals;
    
    const auto temp_dir = std::filesystem::temp_directory_path();
    
    // These are different because Windows is
    // case-insensitive
#if defined(OSLIB_OS_POSIX)
    constexpr size_t tmp_len = 16;
    constexpr std::string_view tmp_template = "tmpXXXXXXXXXXXXXXXX";
#elif defined(OSLIB_OS_WIN32)
    constexpr size_t tmp_len = 18;
    constexpr std::string_view tmp_template = "tmpXXXXXXXXXXXXXXXXXX";
#endif
    auto tmp_name = std::string(tmp_template) + std::string(extension);
    FILE* res = nullptr;
    do {
      temp_chars(std::span<char, tmp_len>(&tmp_name[3], tmp_len));
      const auto test_path = temp_dir/tmp_name;
      
      res = fopen(test_path.string().c_str(), "w+xb");
      if (res == nullptr && !std::filesystem::exists(test_path)) {
#if defined(OSLIB_OS_POSIX)
        try {
          throw std::system_error(errno, std::generic_category());
        }
        catch (...) {
          std::throw_with_nested(std::runtime_error("fopen() failed creating tempfile"));
        }
#elif defined(OSLIB_OS_WIN32)
        throw std::runtime_error("fopen() failed");
#endif
      }
    } while (res == nullptr);
    
    return res;
  }
}  // namespace oslib
#endif