#ifndef OSLIB_SEMAPHORE_HPP
#define OSLIB_SEMAPHORE_HPP


#include "preproc.hpp"

namespace oslib {
  class semaphore;
}

#if defined(OSLIB_OS_POSIX)
#pragma region POSIX semaphore implementation

#include <semaphore.h>

namespace oslib {
  class semaphore {
    sem_t semaphore;
  };
}

#pragma endregion
#elif defined(OSLIB_OS_WIN32)

#endif


#endif