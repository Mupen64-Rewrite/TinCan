#include "secrand.hpp"

#include <exception>
#include <stdexcept>
#include <system_error>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <fcntl.h>
#include <unistd.h>
namespace tc {
  secure_random_device::secure_random_device() : m_hnd(open("/dev/urandom", O_RDONLY | O_CLOEXEC)) {
    if (m_hnd == -1)
      throw std::system_error(errno, std::system_category());
  }
  secure_random_device::~secure_random_device() {
    if (m_hnd >= 0 && close(m_hnd) == -1)
      std::terminate();
  }
  secure_random_device::result_type secure_random_device::operator()() {
    result_type res = 0;
    read(m_hnd, &res, sizeof(res));
    if (m_hnd == -1)
      throw std::system_error(errno, std::system_category());
    return res;
  }
}
#else
#define NOMINMAX
#include <windows.h>
#include <ntstatus.h>
#include <bcrypt.h>
namespace tc {
  secure_random_device::secure_random_device() : m_hnd(invalid_handle) {
    switch (BCryptOpenAlgorithmProvider(&m_hnd, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0)) {
    case STATUS_SUCCESS:
      break;
    case STATUS_NOT_FOUND:
      throw std::runtime_error("BCryptOpenAlgorithmProvider could not find algorithm");
    case STATUS_INVALID_PARAMETER:
      throw std::runtime_error("BCryptOpenAlgorithmProvider failed to get parameter");
    case STATUS_NO_MEMORY:
      throw std::bad_alloc();
    }
  }
  secure_random_device::~secure_random_device() {
    if (m_hnd != invalid_handle && BCryptCloseAlgorithmProvider(m_hnd, 0) != STATUS_SUCCESS)
      std::terminate();
  }
  secure_random_device::result_type secure_random_device::operator()() {
    result_type res = 0;
    BCryptGenRandom(m_hnd, reinterpret_cast<PUCHAR>(&res), sizeof(res), 0);
    return res;
  }
}
#endif