#ifndef OSLIB_SECRAND_HPP
#define OSLIB_SECRAND_HPP


#include <cstdint>
#include <span>
#include <random>
#include <system_error>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>
#undef min
#undef max

#endif
namespace tc {
  class secure_random_device {
  public:
    using result_type = uint32_t;
  
    secure_random_device();
    
    ~secure_random_device();
      
    secure_random_device(const secure_random_device&) = delete;
    secure_random_device& operator=(const secure_random_device&) = delete;
    
    inline secure_random_device(secure_random_device&& rhs) : m_hnd(rhs.m_hnd)  {
      rhs.m_hnd = invalid_handle;
    }
    inline secure_random_device& operator=(secure_random_device&& rhs) {
      m_hnd = rhs.m_hnd;
      rhs.m_hnd = invalid_handle;
      return *this;
    }
    
    // Fetch a random integer.
    result_type operator()();
    
    static constexpr result_type min() {
      return std::numeric_limits<result_type>::min();
    }
    
    static constexpr result_type max() {
      return std::numeric_limits<result_type>::max();
    }
  private:
#if defined(__linux__) || defined(__MACH__)
    int m_hnd;
    static constexpr int invalid_handle = -1;
#elif defined(_WIN32)
    BCRYPT_ALG_HANDLE m_hnd;
    static constexpr BCRYPT_ALG_HANDLE invalid_handle = 0;
#endif
  };
}
#endif