#include "postbox.hpp"

namespace tc {
  void postbox::event_loop(const std::stop_token& stop) {
    using namespace std::literals;

    bool did_anything = false;
    zmq::message_t msg {};

    try {
      if (auto res = m_sock.recv(msg, zmq::recv_flags::dontwait);
          res.has_value()) {
        did_anything = true;
        // unpack data
        msgpack::object_handle data;
        msgpack::unpack(data, (const char*) msg.data(), msg.size());

        // ensure correct formatting
        if (data->type != msgpack::type::object_type::ARRAY)
          throw std::runtime_error("Bad format (!Array.isArray(root))");
        if (data->via.array.size != 2)
          throw std::runtime_error("Bad format (root.length != 2)");

        // Find destination
        auto dest      = data->via.array.ptr[0].as<std::string_view>();
        auto& args_obj = data->via.array.ptr[1];
        // trigger awaiters
        {
          std::shared_lock _lock_(m_await_mutex);
          if (!m_awaiters.empty()) {
            for (auto& awaiter : m_awaiters) {
              if (awaiter.m_event != dest)
                continue;
              if (!awaiter.m_acceptor(args_obj))
                continue;
              awaiter.m_gate.unlock();
            }
          }
        }
        // trigger listener
        if (auto it = m_listeners.find(dest); it != m_listeners.end()) {
          it->second(args_obj);
        }
      }
    }
    catch (...) {
      // do nothing for now
    }
    try {
      if (!m_to_send.empty()) {
        did_anything = true;

        // Message was serialized before enqueuing, just send it
        zmq::message_t to_send;
        m_to_send.pop_return(to_send);
        auto res = m_sock.send(to_send, zmq::send_flags::none);
      }
    }
    catch (...) {
      // do nothing for now
    }

    if (!did_anything && !stop.stop_requested()) {
      std::this_thread::yield();
    }
  }
}  // namespace tc