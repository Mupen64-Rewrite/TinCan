#ifndef TINCAN_IPC_MESSAGE_HPP_INCLUDED
#define TINCAN_IPC_MESSAGE_HPP_INCLUDED
#include <cstdint>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>
#include "function_ref.hpp"
#include <zmq.hpp>
namespace tincan::ipc {
  constexpr void* ptr_offset(void* p, int n) {
    return static_cast<char*>(p) + n;
  }
  constexpr const void* ptr_offset(const void* p, int n) {
    return static_cast<const char*>(p) + n;
  }

  /**
   * Enum identifying the type of event sent
   */
  enum class event_type : uint32_t {

  };
  
  template <event_type ET>
  struct event_for_type;
  
  template <event_type ET>
  using event_for_type_t = typename event_for_type<ET>::type;
  
  #pragma region Event definitions
  
  #pragma endregion

  struct message_header {
    event_type type;
  };

  using server_endpoint = tl::function_ref<void(std::span<char> data)>;

  // singleton ZeroMQ context on either side
  inline zmq::context_t& zmq_context() {
    static zmq::context_t ctx;
    return ctx;
  }

  class pub_socket {
  public:
    pub_socket() : m_sock(zmq_context(), zmq::socket_type::pub) {}

    template <class T>
    void publish(event_type etype, T* data) {
      static_assert(
        std::is_trivially_copyable_v<T>,
        "Cannot send a non-trivially copyable type");

      zmq::message_t msg(sizeof(message_header) + sizeof(T));
      // copy message header
      message_header* mhead = static_cast<message_header*>(msg.data());
      mhead->type           = etype;
      // copy data
      memcpy(ptr_offset(msg.data(), sizeof(message_header)), data, sizeof(T));

      m_sock.send(std::move(msg), zmq::send_flags::none);
    }

  private:
    zmq::socket_t m_sock;
  };

  class sub_socket {
  public:
    sub_socket() : m_sock(zmq_context(), zmq::socket_type::pub) {}

    void subscribe(event_type etype, const server_endpoint& endpoint) {
      endpoints.emplace(etype, endpoint);
    }

    bool try_receive() {
      zmq::message_t msg;
      auto res = m_sock.recv(msg, zmq::recv_flags::dontwait);
      if (!res)
        return false;

      message_header* mhead = static_cast<message_header*>(msg.data());
      char* data =
        static_cast<char*>(ptr_offset(msg.data(), sizeof(message_header)));
      endpoints.at(mhead->type)(
        std::span<char>(data, res.value() - sizeof(message_header)));
    }

    void block_and_receive() {
      zmq::message_t msg;
      auto res = m_sock.recv(msg, zmq::recv_flags::dontwait);
      if (!res)
        return;

      message_header* mhead = static_cast<message_header*>(msg.data());
      char* data =
        static_cast<char*>(ptr_offset(msg.data(), sizeof(message_header)));
      endpoints.at(mhead->type)(
        std::span<char>(data, res.value() - sizeof(message_header)));
    }

  private:
    zmq::socket_t m_sock;
    std::unordered_map<event_type, server_endpoint> endpoints;
  };
}  // namespace tincan::ipc
#endif