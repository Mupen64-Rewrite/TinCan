#ifndef OSLIB_SEMAPHORE_HPP
#define OSLIB_SEMAPHORE_HPP


#include <stdexcept>
#include <system_error>
#include "preproc.hpp"

namespace oslib {
  class ipc_mutex;
}

#if defined(OSLIB_OS_POSIX)
#pragma region POSIX implementation using sem_t

#include <unistd.h>
#include <pthread.h>
#include <cstdio>
#include <cstddef>

namespace oslib {
  
  // Contains the actual semaphore in SHM.
  class ipc_mutex {
  public:
    ipc_mutex() {
      p_check(pthread_mutex_init(&m_mutex, p_init_attrs()));
    }
    
    ~ipc_mutex() {
      p_check(pthread_mutex_destroy(&m_mutex));
    }
    
    void lock() {
      p_check(pthread_mutex_lock(&m_mutex));
    }
    
    void unlock() {
      int res = pthread_mutex_unlock(&m_mutex);
      if (res != 0) {
        perror("ipc_mutex::unlock failed");
      }
    }
    
    void try_lock() {
      p_check(pthread_mutex_trylock(&m_mutex));
    }
    
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
}

#pragma endregion
#elif defined(OSLIB_OS_WIN32)
#pragma region Win32 

namespace oslib {
  class ipc_mutex {
    
  };
}

#pragma endregion
#endif


#endif