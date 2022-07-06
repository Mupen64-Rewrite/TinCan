#ifndef TNP_SHARED_DATA_HPP_INCLUDED
#define TNP_SHARED_DATA_HPP_INCLUDED

#include <google/protobuf/message.h>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include "tnp_ipc.pb.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#if defined(__linux__) || defined(__APPLE__)
  #include <boost/interprocess/shared_memory_object.hpp>
#elif defined(_WIN32)
  #include <boost/interprocess/windows_shared_memory.hpp>
#endif
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace tnp::ipc {
  template <uint32_t max_len>
  class pbuf_msg {
  public:
    pbuf_msg(const tnp::ipc::MessageQuery& msg) : m_len(msg.ByteSizeLong()), m_is_reply(false) {
      if (m_len > max_len) {
        throw std::out_of_range("Message is too long to serialize");
      }
      msg.SerializeToArray(m_data.data(), m_len);
    }
    pbuf_msg(const tnp::ipc::MessageReply& msg) : m_len(msg.ByteSizeLong()), m_is_reply(true) {
      if (m_len > max_len) {
        throw std::out_of_range("Message is too long to serialize");
      }
      msg.SerializeToArray(m_data.data(), m_len);
    }

    void deserialize(google::protobuf::Message& msg) {
      msg.ParseFromArray(m_data.data(), m_len);
    }
    
    bool is_reply() {
      return m_is_reply;
    }

  private:
    uint32_t m_len;
    bool m_is_reply;
    std::array<std::byte, max_len> m_data;
  };

  template <class T, size_t S>
  class shared_blocking_queue {
    static_assert(
      S != 0, "shared_blocking_queue requires at least 1 element of space");

  public:
    shared_blocking_queue() :
      m_head(0), m_len(0), m_sem_remain(S), m_sem_filled(0), m_mutex() {}
      
    ~shared_blocking_queue() {
      size_t end = (m_head + m_len) % S;
      for (size_t i = m_head; i != end; i = (i + 1) % S) {
        m_data[i].value.~T();
      }
    }

    void send(const T& value) {
      m_sem_remain.wait();
      {
        std::scoped_lock _lock(m_mutex);

        new (&m_data[(m_head + m_len) % S]) T(value);
        m_len += 1;
      }
      m_sem_filled.post();
    }

    void send(T&& value) {
      m_sem_remain.wait();
      {
        std::scoped_lock _lock(m_mutex);

        new (&m_data[(m_head + m_len) % S].value) T(value);
        m_len += 1;
      }
      m_sem_filled.post();
    }

    template <class... Args>
    requires std::is_constructible_v<T, Args...>
    void emplace(Args&&... args) {
      m_sem_remain.wait();
      {
        std::scoped_lock _lock(m_mutex);

        new (&m_data[(m_head + m_len) % S].value) T(std::forward<Args>(args)...);
        m_len += 1;
      }
      m_sem_filled.post();
    }

    T receive() {
      std::optional<T> res;
      m_sem_filled.wait();
      {
        std::scoped_lock _lock(m_mutex);
        res = std::move(m_data[m_head].value);
        m_data[m_head].value.~T();
        
        m_head = (m_head + 1) % S;
        m_len -= 1;
      }
      m_sem_remain.post();

      return *res;
    }

  private:
    union element_data {
      char dummy = 0;
      T value;
      
      element_data() : dummy(0) {}
    };
  
    std::array<element_data, S> m_data;
    size_t m_head;
    size_t m_len;
    boost::interprocess::interprocess_semaphore m_sem_remain;
    boost::interprocess::interprocess_semaphore m_sem_filled;
    boost::interprocess::interprocess_mutex m_mutex;
  };
}  // namespace tnp::ipc

#endif