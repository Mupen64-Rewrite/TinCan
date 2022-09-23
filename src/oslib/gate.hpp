#ifndef OSLIB_SIGNALER_HPP_INCLUDED
#define OSLIB_SIGNALER_HPP_INCLUDED

#include "preproc.hpp"

namespace oslib {
  // A gate synchronization primitive.
  // When initialized, it is closed. Threads that call wait()
  // will block until the gate is opened.
  class ipc_gate;
}
  #if defined(OSLIB_OS_LINUX)
  #pragma region Linux futex implementation
  
  #include <sys/syscall.h>
  #include <linux/futex.h>
  #include <unistd.h>
  #include <atomic>
  #include <cstdint>
  #include <limits>
  #include <thread>
  
namespace oslib {
  class ipc_gate {
  public:
    ipc_gate() : m_value(1), m_exiting(false) {}
    
    ~ipc_gate() {
      m_exiting = true;
      syscall(SYS_futex, &m_value, FUTEX_WAKE, std::numeric_limits<uint32_t>::max());
    }
    
    void wait() {
      // spin a few times before sleeping
      for (uint32_t i = 0; i < 4; i++) {
        if (m_value == 0 || m_exiting)
          return;
        std::this_thread::yield();
      }
      while (m_value == 1 && !m_exiting)
        syscall(SYS_futex, &m_value, FUTEX_WAIT, 1, nullptr);
    }
    
    void open() {
      m_value = 0;
      syscall(SYS_futex, &m_value, FUTEX_WAKE, std::numeric_limits<uint32_t>::max());
    }
    
    void close() {
      m_value = 1;
    }
  private:
    std::atomic_uint32_t m_value;
    bool m_exiting;
  };
}
  #pragma endregion
  #elif defined(OSLIB_OS_POSIX)
  #pragma region POSIX implementation
  #include <atomic>
  #include <pthread.h>
namespace oslib {
  // Slow implementation of a gate.
  // Threads must exit one-by-one, instead of simultaneously.
  class ipc_gate {
  public:
    ipc_gate() {
      p_check(pthread_mutex_init(&m_mutex, p_init_mutexattrs()));
      p_check(pthread_cond_init(&m_cond, p_init_condattrs()));
    }
    
    ~ipc_gate() {
      p_check(pthread_mutex_destroy(&m_mutex));
      p_check(pthread_cond_destroy(&m_cond));
    }
  
    void wait() {
      if (m_flag)
        return;
      p_check(pthread_mutex_lock(&m_mutex));
      while (!m_flag) {
        p_check(pthread_cond_wait(&m_cond, &m_mutex));
      }
      p_check(pthread_mutex_unlock(&m_mutex));
    }
    void open() {
      m_flag = true;
      p_check(pthread_cond_broadcast(&m_cond));
    }
    void close() {
      m_flag = false;
    }
  private:
    static pthread_mutexattr_t* p_init_mutexattrs() {
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
    static pthread_condattr_t* p_init_condattrs() {
      static struct _local_builder_t {
        _local_builder_t() {
          p_check(pthread_condattr_init(&obj));
          p_check(pthread_condattr_setpshared(&obj, PTHREAD_PROCESS_SHARED));
        }

        pthread_condattr_t obj;
      } _local_builder {};

      return &_local_builder.obj;
    }

    static inline void p_check(int res, int n = 0) {
      if (res != n)
        throw std::system_error(errno, std::system_category());
    }
  
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    std::atomic_bool m_flag;
  };
}
  #pragma endregion
  #endif
#endif