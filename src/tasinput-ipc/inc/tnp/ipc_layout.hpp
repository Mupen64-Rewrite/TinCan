#ifndef TNP_IPC_LAYOUT_HPP_INCLUDED
#define TNP_IPC_LAYOUT_HPP_INCLUDED

#include <atomic>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <tnp/ipc_types.hpp>

#if defined(__linux__) || defined(__APPLE__)
  #include <boost/interprocess/shared_memory_object.hpp>
#elif defined(_WIN32)
  #include <boost/interprocess/windows_shared_memory.hpp>
#endif

#include <iomanip>
#include <random>
#include <sstream>

namespace tnp {
  struct ipc_layout {
    std::atomic_uint64_t counter = 0;
    std::array<uint32_t, 4> ctrl_state;
    tnp::ipc::shared_blocking_queue<tnp::ipc::pbuf_msg<512>, 8> mq_p2e;
    tnp::ipc::shared_blocking_queue<tnp::ipc::pbuf_msg<512>, 8> mq_e2p;
  };

#if defined(__linux__) || defined(__APPLE__)
  using native_shm_type = boost::interprocess::shared_memory_object;
  class shm_server {
  public:
    shm_server() :
      m_id(generate_id("tasinput-qt")),
      m_shm(
        (native_shm_type::remove(m_id.c_str()),
         native_shm_type(
           boost::interprocess::create_only, m_id.c_str(),
           boost::interprocess::read_write))),
      m_region(
        (void(m_shm.truncate(sizeof(ipc_layout))),
         boost::interprocess::mapped_region(
           m_shm, boost::interprocess::read_write))) {
      new (m_region.get_address()) ipc_layout;        
    }

    ~shm_server() { 
      reinterpret_cast<ipc_layout*>(m_region.get_address())->~ipc_layout();
      native_shm_type::remove(m_id.c_str()); 
    }

    ipc_layout& ipc_data() {
      return *reinterpret_cast<ipc_layout*>(m_region.get_address());
    }
    
    const std::string& id() {
      return m_id;
    }

  private:
    static std::string generate_id(std::string_view base) {
      std::random_device rd;
      uint64_t id = (uint64_t(rd()) << 32) | uint64_t(rd());

      std::ostringstream oss;
      oss << base << '_' << std::hex << std::setw(16) << std::setfill('0')
          << std::uppercase << id;
      return oss.str();
    }

    std::string m_id;
    native_shm_type m_shm;
    boost::interprocess::mapped_region m_region;
  };

  class shm_client {
  public:
    shm_client(std::string_view server_id) :
      m_id(server_id),
      m_shm(
        boost::interprocess::open_only, m_id.c_str(),
        boost::interprocess::read_write),
      m_region(m_shm, boost::interprocess::read_write) {
    }

    ipc_layout& ipc_data() {
      return *reinterpret_cast<ipc_layout*>(m_region.get_address());
    }

  private:
    std::string m_id;
    native_shm_type m_shm;
    boost::interprocess::mapped_region m_region;
  };
#elif defined(_WIN32)
  using native_shm_type = boost::interprocess::windows_shared_memory;
#endif
}  // namespace tnp

#endif