#ifndef OSLIB_SHMEM_HPP_INCLUDED
#define OSLIB_SHMEM_HPP_INCLUDED

#include <cstddef>

#include <cstdlib>
#include <iostream>
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

namespace oslib {
  // Shared memory with an inheritable handle.
  struct shm_object {
  public:
    using native_handle_type = int;

    // Creates a shm_object.
    shm_object(size_t size) : m_fd(acquire_shm_fd()), m_size(size) {
      if (m_fd == -1) {
        throw std::system_error(errno, std::generic_category());
      }
      ftruncate(m_fd, size);
    }

    // Opens an existing shared memory file descriptor.
    shm_object(native_handle_type fd, size_t size) : m_fd(fd), m_size(size) {
      // if (fcntl(F_GETFD, fd) == -1) {
      //   throw std::system_error(errno, std::generic_category());
      // }
    }

    ~shm_object() {
      close(m_fd);
    }

    shm_object(const shm_object&)            = delete;
    shm_object& operator=(const shm_object&) = delete;
    
    shm_object(shm_object&&) = default;
    shm_object& operator=(shm_object&&) = default;

    // Returns the native handle.
    // The shm_object's handle should be inheritable from the child process,
    // allowing it to be passed to `shm_object::shm_object(native_handle_type fd,
    // size_t size)`
    native_handle_type native_handle() {
      return m_fd; }

    size_t size() { return m_size; }
    
    // Maps the shm object in the current process.
    shm_mapping map();

  private:
    // Private method: get ahold of an anonymous shared-memory FD.
    // This is accomplished by memfd_create on Linux, and shm_open on other Unices.
    static int acquire_shm_fd();

    int m_fd;
    size_t m_size;
  };

  #if defined(OSLIB_OS_LINUX)
  inline int shm_object::acquire_shm_fd() {
    // Call memfd_create on Linux. This can operate under
    // the same semantics as a shm_open'd FD.
    char name[] = "tasinput-memfd-############.shm";
    mkstemp(name);
    
    return memfd_create(name, 0);
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
    shm_mapping(const shm_mapping&) = delete;
    shm_mapping& operator=(const shm_mapping&) = delete;
    
    shm_mapping(shm_mapping&& rhs) : base(rhs.base), size(rhs.size) {
      rhs.base = nullptr;
    }
    shm_mapping& operator=(shm_mapping&& rhs) {
      base = rhs.base;
      size = rhs.size;
      rhs.base = nullptr;
      
      return *this;
    }

    // Returns a pointer to an address within the block of shared memory.
    template <class T>
    T addr(size_t addr = 0x0) {
      static_assert(
        std::is_pointer_v<T> &&
          (std::is_object_v<std::remove_pointer_t<T>> ||
           std::is_void_v<std::remove_pointer_t<T>>),
        "T must be a pointer to an instantiable type");

      bool bounds_check = addr >= size;
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
        
      bool bounds_check = addr >= size;
      if (bounds_check) {
        throw std::out_of_range(
          "Address {} is out of bounds for region of size {}");
      }

      return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(base) + addr);
    }

    ~shm_mapping() {
      if (base != nullptr)
        munmap(base, size); 
    }

  private:
    shm_mapping(void* p, size_t size) : base(p), size(size) {}

    void* base;
    size_t size;
  };

  inline shm_mapping shm_object::map() {
    void* p = mmap(0, size(), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (p == MAP_FAILED) {
      throw std::system_error(errno, std::generic_category());
    }
    return shm_mapping(p, size());
  }
}  // namespace oslib
  #pragma endregion
#elif defined(OSLIB_OS_WIN32)
  #pragma region WinAPI implementation
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef _WINSOCKAPI_
    #define _WINSOCKAPI_
  #endif
  #include <windows.h>
   // Shared memory with an inheritable handle.
struct shm_object {
public:
  using native_handle_type = HANDLE;

  // Creates a shm_object.
  shm_object(size_t size) :
    map_handle(acquire_mapping_handle(size)), size(size) {}

  // Opens an existing shared memory file descriptor.
  shm_object(native_handle_type hnd, size_t size) : map_handle(hnd), size(size) {}

  shm_object(const shm_object&) = delete;
  shm_object& operator=(const shm_object&) = delete;
    
  shm_mapping(shm_mapping&&) = default;
  shm_mapping& operator=(shm_mapping&&) = delete;

  ~shm_object() { CloseHandle(map_handle); }

  // Returns the native handle.
  // The shm_object's handle should be inheritable from the child process,
  // allowing it to be passed to `shm_object::shm_object(native_handle_type fd,
  // size_t size)`
  native_handle_type native_handle() { return map_handle; }

  size_t size() { return size; }

  // Maps the shm object in the current process.
  shm_mapping map();

private:
  // Private method: get ahold of a HANDLE to file mapping.
  static HANDLE acquire_mapping_handle(size_t size);

  HANDLE map_handle;
  size_t size;
};
inline HANDLE shm_object::acquire_mapping_handle(size_t size) {
  static const SECURITY_ATTRIBUTES attrs = {
    .nLength              = sizeof(SECURITY_ATTRIBUTES),
    .lpSecurityDescriptor = nullptr,
    .bInheritHandle       = true};

  DWORD dwFMSizeLow  = size & 0xFFFFFFFF;
  DWORD dwFMSizeHigh = 0;
  if constexpr (sizeof(size_t) > 4) {
    dwFMSizeHigh = (size >> 32) & 0xFFFFFFFF;
  }

  // clang-format off
  HANDLE res = CreateFileMappingW(
    INVALID_HANDLE_VALUE, &attrs, PAGE_READWRITE, 
    dwFMSizeHigh, dwFMSizeLow, nullptr);
  // clang-format on

  if (res == nullptr) {
    throw std::system_error(GetLastError(), std::system_category());
  }

  return res;
}

struct shm_mapping {
  friend class shm_object;

public:
  shm_mapping(const shm_mapping&) = delete;
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

  ~shm_mapping() { UnmapViewOfFile(base); }

private:
  shm_mapping(void* p, size_t size) : base(p), size(size) {}

  void* base;
  size_t size;
};

inline shm_mapping shm_object::map() {
  LPVOID res = MapViewOfFile(map_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (res == nullptr) {
    throw std::system_error(GetLastError(), std::system_category());
  }
  return shm_mapping {res, size};
}
  #pragma endregion
#endif

#endif