#ifndef OSLIB_SHMEM_HPP_INCLUDED
#define OSLIB_SHMEM_HPP_INCLUDED

#include <cstddef>

#include <source_location>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <version>

#include "preproc.hpp"

namespace oslib {
  struct shm_object;
  struct shm_mapping;
}  // namespace oslib

#if defined(OSLIB_OS_POSIX)
  #pragma region Generally POSIX implementation

  #if defined(OSLIB_OS_LINUX)
    #include <linux/memfd.h>
    #include <linux/unistd.h>
  #endif

  #include <fcntl.h>
  #include <sys/mman.h>
  #include <unistd.h>

  #include <cerrno>
  #include <cstdint>
  #include <ctime>

  #include "details/meta.hpp"

namespace oslib {
  // Shared memory with an inheritable handle.
  struct shm_object {
  public:
    using native_handle_type = int;

    // Creates a shm_object.
    shm_object(size_t size) : fd(acquire_shm_fd()) {
      if (fd == -1) {
        throw std::system_error(errno, std::generic_category());
      }
      ftruncate(fd, size);
    }

    // Opens an existing shared memory file descriptor.
    shm_object(native_handle_type fd, size_t size) : fd(fd), size(size) {
      if (fcntl(F_GETFD, fd) == -1) {
        throw std::system_error(errno, std::generic_category());
      }
    }

    shm_object(const shm_object&)            = delete;
    shm_object& operator=(const shm_object&) = delete;

    // Returns the native handle.
    // The shm_object's handle should be inheritable from the child process,
    // allowing it to be passed to `shm_object::shm_object(native_handle_type fd,
    // size_t size)`
    native_handle_type native_handle() { return fd; }
    
    // Maps the shm object in the current process.
    shm_mapping map();

  private:
    // Private method: get ahold of an anonymous shared-memory FD.
    // This is accomplished by memfd_create on Linux, and shm_open on other Unices.
    static int acquire_shm_fd();

    int fd;
    size_t size;
  };

  #if defined(OSLIB_OS_LINUX)
  inline int shm_object::acquire_shm_fd() {
    // Call memfd_create on Linux. This can operate under
    // the same semantics as a shm_open'd FD.
    return syscall(__NR_memfd_create, (unsigned int) 0);
  }
  #else
  inline int shm_object::acquire_shm_fd() {
    // Try to generate a name, shm_open it, then shm_unlink it.
    // This is the only (mostly) POSIX-compliant solution.

    char name[16]   = "/shm-";
    char* const end = name + sizeof(name);
    struct timespec tv;
    int fd;

    for (int attempts = 0; attempts < 4; attempts++) {
      // this guarantees uniqueness on a single core
      clock_gettime(CLOCK_REALTIME, &tv);
      // custom hash thing, i guess
      uint64_t hash = (((uint64_t) tv.tv_sec) * 131071) + (uint64_t) tv.tv_nsec;

      for (char* i = name; i < end; (i++, hash /= 8)) {
        *i = '0' + (hash % 8);
      }
      // Try to shm_open with this name, then unlink it
      fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
      if (fd != -1) {
        if (shm_unlink(name) == -1) {
          int tmp = errno;
          close(fd);
          errno = tmp;

          return -1;
        }

        int old_fl = fcntl(fd, F_GETFL);
        if (old_fl == -1) {
          return -1;
        }
        // Unset the close-on-`exec` flag, since we want
        // a child process to inherit this FD.
        // This might not be POSIX-compliant, but someone
        // tested this on MacOS and Linux and it works
        if (fcntl(fd, F_SETFL, old_fl & ~O_CLOEXEC) == -1) {
          return -1;
        }
        return fd;
      }
      if (errno != EEXIST) {
        return -1;
      }
    }

    return -1;
  }
  #endif

  struct shm_mapping {
    friend class shm_object;

  public:
    shm_mapping(const shm_mapping&)            = delete;
    shm_mapping& operator=(const shm_mapping&) = delete;

    // Returns a pointer to an address within the block of shared memory.
    template <class T>
    T addr(size_t addr = 0x0) {
      static_assert(
        std::is_pointer_v<T> &&
          (std::is_object_v<std::remove_pointer_t<T>> ||
           std::is_void_v<std::remove_pointer_t<T>>),
        "T must be a pointer to an instantiable type");

      bool bounds_check = addr >= size;
      if constexpr (!OSLIB_DETAILS_META_IS_COMPLETE(T)) {
        bounds_check |= addr + sizeof(T) >= size;
      }
      if (bounds_check) {
        throw std::out_of_range(
          "Address {} is out of bounds for region of size {}");
      }

      return reinterpret_cast<T>(reinterpret_cast<std::byte*>(base) + addr);
    }
    
    // Gets a reference to an object in the block of shared memory.
    template <class T>
    T& read(size_t addr = 0x0) {
      static_assert(
        std::is_object_v<T>, "T must be a pointer to instantiable type");

      if (addr + sizeof(T) >= size) {
        throw std::out_of_range(
          "Address {} is out of bounds for region of size {}");
      }

      return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(base) + addr);
    }

    ~shm_mapping() { munmap(base, size); }

  private:
    shm_mapping(void* p, size_t size) : base(p), size(size) {}

    void* base;
    size_t size;
  };

  inline shm_mapping shm_object::map() {
    void* p = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
      throw std::system_error(errno, std::generic_category());
    }
    return shm_mapping(p, size);
  }
}  // namespace oslib
  #pragma endregion
#elif defined(OSLIB_OS_WINDOWS)
  #pragma region WinAPI implementation
  #include <windows.h>
  // Shared memory with an inheritable handle.
  struct shm_object {
  public:
    using native_handle_type = HANDLE;

    // Creates a shm_object.
    shm_object(size_t size) {
    }

    // Opens an existing shared memory file descriptor.
    shm_object(native_handle_type fd, size_t size) {
    }

    shm_object(const shm_object&)            = delete;
    shm_object& operator=(const shm_object&) = delete;

    // Returns the native handle.
    // The shm_object's handle should be inheritable from the child process,
    // allowing it to be passed to `shm_object::shm_object(native_handle_type fd,
    // size_t size)`
    native_handle_type native_handle() { return fd; }
    
    // Maps the shm object in the current process.
    shm_mapping map();

  private:
    // Private method: get ahold of an anonymous shared-memory FD.
    // This is accomplished by memfd_create on Linux, and shm_open on other Unices.
    static int acquire_shm_fd();

    HANDLE mapHandle;
    size_t size;
  };
  inline int shm_object::acquire_shm_fd() {
  }

  struct shm_mapping {
    friend class shm_object;

  public:
    shm_mapping(const shm_mapping&)            = delete;
    shm_mapping& operator=(const shm_mapping&) = delete;

    // Returns a pointer to an address within the block of shared memory.
    template <class T>
    T addr(size_t addr = 0x0) {
      static_assert(
        std::is_pointer_v<T> &&
          (std::is_object_v<std::remove_pointer_t<T>> ||
           std::is_void_v<std::remove_pointer_t<T>>),
        "T must be a pointer to an instantiable type");

      bool bounds_check = addr >= size;
      if constexpr (!OSLIB_DETAILS_META_IS_COMPLETE(T)) {
        bounds_check |= addr + sizeof(T) >= size;
      }
      if (bounds_check) {
        throw std::out_of_range(
          "Address {} is out of bounds for region of size {}");
      }

      return reinterpret_cast<T>(reinterpret_cast<std::byte*>(base) + addr);
    }
    
    // Gets a reference to an object in the block of shared memory.
    template <class T>
    T& read(size_t addr = 0x0) {
      static_assert(
        std::is_object_v<T>, "T must be a pointer to instantiable type");

      if (addr + sizeof(T) >= size) {
        throw std::out_of_range(
          "Address {} is out of bounds for region of size {}");
      }

      return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(base) + addr);
    }

    ~shm_mapping() {  }

  private:
    shm_mapping(void* p, size_t size) : base(p), size(size) {}

    void* base;
    size_t size;
  };

  inline shm_mapping shm_object::map() {
  }
  #pragma endregion
#endif

#endif