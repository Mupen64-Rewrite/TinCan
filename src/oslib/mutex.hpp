#ifndef OSLIB_SEMAPHORE_HPP
#define OSLIB_SEMAPHORE_HPP

#include <stdexcept>
#include <system_error>
#include <exception>
#include "preproc.hpp"

namespace oslib {
  class ipc_mutex;
}

#if defined(OSLIB_OS_POSIX)
  #pragma region POSIX implementation using sem_t

  #include <pthread.h>
  #include <unistd.h>
  #include <cstddef>
  #include <cstdio>

namespace oslib {

  // Contains the actual semaphore in SHM.
  class ipc_mutex {
  public:
    ipc_mutex() { p_check(pthread_mutex_init(&m_mutex, p_init_attrs())); }

    ~ipc_mutex() { p_check(pthread_mutex_destroy(&m_mutex)); }

    // Call from the client to clean up the mutex.
    // Does ABSOLUTELY NOTHING on POSIX where mutexes are only destroyed once.
    void client_cleanup() noexcept {}

    void lock() { p_check(pthread_mutex_lock(&m_mutex)); }

    void unlock() {
      int res = pthread_mutex_unlock(&m_mutex);
      if (res != 0) {
        perror("ipc_mutex::unlock failed");
        std::terminate();
      }
    }

    void try_lock() { p_check(pthread_mutex_trylock(&m_mutex)); }

  private:
    static pthread_mutexattr_t* p_init_attrs() {
      static struct _local_builder_t {
        _local_builder_t() {
          p_check(pthread_mutexattr_init(&obj));
          p_check(pthread_mutexattr_setpshared(&obj, PTHREAD_PROCESS_SHARED));
          p_check(pthread_mutexattr_setrobust(&obj, PTHREAD_MUTEX_ROBUST));
        }

        pthread_mutexattr_t obj;
      } _local_builder {};

      return &_local_builder.obj;
    }

    static inline void p_check(int res, int n = 0) {
      if (res != n)
        throw std::system_error(errno, std::system_category());
    }

    pthread_mutex_t m_mutex;
  };
}  // namespace oslib

  #pragma endregion
#elif defined(OSLIB_OS_WIN32)
  #pragma region Win32 implementation

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdio>


namespace oslib {
  class ipc_mutex {
  public:
    ipc_mutex() : m_hmutex(CreateMutexW(p_init_secattrs(), FALSE, nullptr)) {
      if (m_hmutex == nullptr) {
        throw std::system_error(GetLastError(), std::system_category());
      }
    }

    ~ipc_mutex() { CloseHandle(m_hmutex); }

    // Call from the client to clean up the mutex.
    void client_cleanup() noexcept { CloseHandle(m_hmutex); }

    void lock() { 
      DWORD res = WaitForSingleObject(m_hmutex, INFINITE);
      switch (res) { 
        case WAIT_OBJECT_0:
          break;
        case WAIT_FAILED: {
          throw std::system_error(GetLastError(), std::system_category());
        } break;
        case WAIT_ABANDONED: {
          throw std::runtime_error("Owning process of mutex died");
        } break;
      }
      throw std::runtime_error("This should never happen. Report it as a bug.");
    }

    void unlock() { 
      BOOL res = ReleaseMutex(m_hmutex);
      if (!res) {
        auto err = std::system_error(GetLastError(), std::system_category());
        fprintf(stderr, "ipc_mutex::unlock failed: %s\n", err.what());
        std::terminate();
      }
    }

    bool try_lock() {
      DWORD res = WaitForSingleObject(m_hmutex, 0);
      switch (res) {
        case WAIT_OBJECT_0:
          return true;
        case WAIT_TIMEOUT:
          return false;
        case WAIT_FAILED: {
          throw std::system_error(GetLastError(), std::system_category());
        } break;
        case WAIT_ABANDONED: {
          throw std::runtime_error("Owning process of mutex died");
        } break;
      }
      throw std::runtime_error("This should never happen. Report it as a bug.");
    }

  private:
    static inline LPSECURITY_ATTRIBUTES p_init_secattrs() {
      static struct SECURITY_ATTRIBUTES attrs { 
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = nullptr,
        .bInheritHandle = TRUE,
      };
      return &attrs;
    }

    HANDLE m_hmutex;
  };
}  // namespace oslib

  #pragma endregion
#endif

#endif