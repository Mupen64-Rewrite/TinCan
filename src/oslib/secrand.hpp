#ifndef OSLIB_SECRAND_HPP_INCLUDED
#define OSLIB_SECRAND_HPP_INCLUDED


#include <cstdint>
#include <span>
#include <random>
#include <system_error>

#include "preproc.hpp"

namespace oslib {
  // Like random_device, but guaranteed to be secure.
  class secure_random_device;
}

#if defined(OSLIB_OS_POSIX)
namespace oslib {
  class secure_random_device {
  public:
    using result_type = unsigned int;
  
    secure_random_device() :
      dev("/dev/urandom") {}
      
    secure_random_device(const secure_random_device&) = delete;
    
    // (at least for libstdc++ and libc++)
    // Fetches a random integer from /dev/urandom.
    result_type operator()() {
      return dev();
    }
    
    static constexpr result_type min() {
      return std::random_device::min();
    }
    
    static constexpr result_type max() {
      return std::random_device::max();
    }
  private:
    std::random_device dev;
  };
}
#elif defined(OSLIB_OS_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bit>
#include <limits>

#undef min
#undef max

namespace oslib {
  class secure_random_device {
  public:
    using result_type = unsigned int;

    secure_random_device() {}
    secure_random_device(const secure_random_device&) = delete;
    
    // Uses BCryptGenRandom to generate a random number.
    result_type operator()() { 
      std::array<uint8_t, sizeof(result_type)> bytes;
      BCryptGenRandom(get_bcrypt_alg(), &bytes[0], sizeof(result_type), 0);
      return std::bit_cast<result_type>(bytes);
    }
    static constexpr result_type min() {
      return std::numeric_limits<result_type>::min();
    }
    static constexpr result_type max() { 
      return std::numeric_limits<result_type>::max();
    }
  private:
    static BCRYPT_ALG_HANDLE get_bcrypt_alg() {
      static BCRYPT_ALG_HANDLE res = nullptr;
      if (res == nullptr) {
        auto status = BCryptOpenAlgorithmProvider(
          &res, MS_PRIMITIVE_PROVIDER, nullptr, 0
        );
        switch (status) { 
          case STATUS_SUCCESS:
            break;
          case STATUS_NOT_FOUND:
          case STATUS_INVALID_PARAMETER:
          case STATUS_NO_MEMORY: {
            throw std::runtime_error("BCryptOpenAlgorithmProvider failed");
          } break;
        }
      }
      return res;
    }
  };
}
#endif

#endif