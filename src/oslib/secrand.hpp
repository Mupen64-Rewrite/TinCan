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
namespace oslib {
  class secure_random_device {
  public:
    using result_type = unsigned int;
  };
}
#endif

#endif