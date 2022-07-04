#ifndef TNP_SHARED_DATA_HPP_INCLUDED
#define TNP_SHARED_DATA_HPP_INCLUDED

#include <google/protobuf/message.h>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#if defined(__linux__) || defined(__APPLE__)
  #include <boost/interprocess/shared_memory_object.hpp>
#elif defined(_WIN32)
  #include <boost/interprocess/windows_shared_memory.hpp>
#endif
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace tnp::ipc {
  template <size_t max_len>
  class pbuf_msg {
  public:
    pbuf_msg(const google::protobuf::Message& msg) :
      m_len(msg.ByteSizeLong()) {
      if (m_len > max_len) {
        throw std::out_of_range("Message is too long to serialize");
      }  
      msg.SerializeToArray(m_data.data(), m_len);
    }
    
    void deserialize(google::protobuf::Message& msg) {
      msg.ParseFromArray(m_data.data(), m_len);
    }
    
  private:
    size_t m_len;
    std::array<std::byte, max_len> m_data;
  };
  
  template <class T, size_t S>
  class shared_blocking_queue {
    static_assert(S != 0, "shared_blocking_queue requires at least 1 element of space");
  public:
    shared_blocking_queue() :
      m_head(0),
      m_len(0),
      m_sem_remain(S),
      m_sem_filled(0),
      m_mutex() {}
      
    void send(const T& value) {
      m_sem_remain.wait();
      {
        std::scoped_lock _lock(m_mutex);
        
        m_data[(m_head + m_len) % S] = value;
        m_len += 1;
      }
      m_sem_filled.post();
    }
    
    void send(T&& value) {
      m_sem_remain.wait();
      {
        std::scoped_lock _lock(m_mutex);
        
        m_data[(m_head + m_len) % S] = value;
        m_len += 1;
      }
      m_sem_filled.post();
    }
    
    T receive() {
      std::optional<T> res;
      m_sem_filled.wait();
      {
        std::scoped_lock _lock(m_mutex);
        res = m_data[m_head];
        
        m_head = (m_head + 1) % S;
        m_len -= 1;
      }
      m_sem_remain.post();
      
      return *res;
    }
  private:
    std::array<T, S> m_data;
    size_t m_head;
    size_t m_len;
    boost::interprocess::interprocess_semaphore m_sem_remain;
    boost::interprocess::interprocess_semaphore m_sem_filled;
    boost::interprocess::interprocess_mutex m_mutex;
  };
}  // namespace tnp::ipc

#endif