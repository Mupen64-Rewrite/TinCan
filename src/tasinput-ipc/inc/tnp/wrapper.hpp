#ifndef TNP_SHM_WRAPPER_HPP_INCLUDED
#define TNP_SHM_WRAPPER_HPP_INCLUDED

#include <type_traits>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/intrusive/pointer_traits.hpp>
#if defined(__linux__) || defined(__APPLE__)
  #include <boost/interprocess/shared_memory_object.hpp>
#elif defined(_WIN32)
  #include <boost/interprocess/windows_shared_memory.hpp>
#endif

namespace tnp {
  namespace bip = boost::interprocess;

  class shm_wrapper;

#if defined(__linux__) || defined(__APPLE__)
  class shm_wrapper {
  public:
    shm_wrapper(size_t s, const char* name, bool is_server) :
      is_server(is_server) {
      if (is_server) {
        bip::shared_memory_object::remove(name);
        
        new (&shm_obj)
          bip::shared_memory_object(bip::create_only, name, bip::read_write);
      }
      else {
        new (&shm_obj)
          bip::shared_memory_object(bip::open_only, name, bip::read_write);
      }

      shm_obj.truncate(s);
    }

    ~shm_wrapper() {
      std::string name = shm_obj.get_name();
      if (is_server)
        bip::shared_memory_object::remove(name.c_str());
      shm_obj.bip::shared_memory_object::~shared_memory_object();
    }

    bip::mapped_region map() {
      return bip::mapped_region(shm_obj, bip::read_write);
    }

  private:
    bool is_server;
    // forces manual object lifetime management.
    union {
      char _shm_obj_dummy;
      bip::shared_memory_object shm_obj;
    };
  };
#elif defined(_WIN32)
    // TODO define using windows shm object
#endif
}  // namespace tnp

#endif