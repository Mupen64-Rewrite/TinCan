#ifndef TC_IPC_POSTBOX_HPP
#define TC_IPC_POSTBOX_HPP

#include "../tl/function_ref.hpp"
#include "../util/string_hash.hpp"
#include "../util/gate.hpp"
#include "safe_queue.hpp"

#include <atomic>
#include <concepts>
#include <iostream>
#include <latch>
#include <list>
#include <shared_mutex>
#include <stdexcept>
#include <stop_token>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <fmt/core.h>
#include <msgpack.hpp>
#include <msgpack/v3/object_decl.hpp>
#include <msgpack/v3/sbuffer_decl.hpp>
#include <msgpack/v3/unpack_decl.hpp>
#include <zmq.hpp>

namespace tc {
  inline bool accept_always(const msgpack::object&) {
    return true;
  }

  class postbox {
  public:
    using listener_type = tl::function_ref<void(const msgpack::object&)>;
    using acceptor_type = tl::function_ref<bool(const msgpack::object&)>;

  private:
    struct internal_awaiter {
      internal_awaiter(std::string_view event, const acceptor_type& acceptor) :
        m_event(event), m_acceptor(acceptor), m_gate() {}

      std::string m_event;
      acceptor_type m_acceptor;
      tc::gate m_gate;
    };

    zmq::socket_t m_sock;
    tc::safe_queue<zmq::message_t> m_to_send;
    tc::string_map<listener_type> m_listeners;
    std::list<internal_awaiter> m_awaiters;
    std::shared_mutex m_await_mutex;

  public:
    class await_handle {
      friend class postbox;

    private:
      postbox* m_parent;
      std::list<internal_awaiter>::iterator m_it;

      await_handle(postbox* self, std::list<internal_awaiter>::iterator it) :
        m_parent(self), m_it(it) {}

    public:
      ~await_handle() { 
        if (m_parent)
          remove(); 
      }
      
      await_handle(const await_handle&) = delete;
      await_handle& operator=(const await_handle&) = delete;
      
      await_handle(await_handle&& rhs) : m_parent(rhs.m_parent), m_it(rhs.m_it) {
        rhs.m_parent = nullptr;
      }
      
      await_handle& operator=(await_handle&& rhs) {
        m_parent = rhs.m_parent;
        m_it = rhs.m_it;
        rhs.m_parent = nullptr;
        return *this;
      }

      void remove() noexcept {
        std::scoped_lock lock(m_parent->m_await_mutex);
        m_parent->m_awaiters.erase(m_it);
      }

      bool completed() const noexcept { return m_it->m_gate.try_wait(); }

      void await() const noexcept { m_it->m_gate.wait(); }
    };

    postbox(zmq::context_t& ctx, const char* uri) :
      m_sock(ctx, zmq::socket_type::pair) {
      m_sock.set(zmq::sockopt::sndtimeo, 50);
      m_sock.set(zmq::sockopt::rcvtimeo, 50);
      m_sock.bind(uri);
    }
    postbox(zmq::context_t& ctx, const std::string& uri) :
      postbox(ctx, uri.c_str()) {}

    std::string endpoint() { return m_sock.get(zmq::sockopt::last_endpoint); }

    // Performs 1 iteration of the postbox's event loop.
    void event_loop(const std::stop_token& stop);

    // Enqueues an event to be sent to the other side.
    // This is thread-safe.
    template <class... Args>
    void enqueue(std::string_view event, Args&&... args) {
      msgpack::sbuffer buffer;
      msgpack::packer packer {buffer};

      packer.pack_array(2);
      packer.pack(event);
      packer.pack_array(sizeof...(Args));
      ((packer.pack(std::forward<Args>(args))), ...);

      m_to_send.emplace((void*) buffer.data(), buffer.size());
    }

    // Sets the listener for a particular event. Does nothing if there is
    // already a listener. Do not use while the event loop is active.
    void listen(std::string_view event, const listener_type& listener) {
      m_listeners.emplace(event, listener);
    }

    // Sets the listener for a particular event. Does nothing if there is
    // already a listener. Do not use while the event loop is active.
    void listen(std::string_view event, listener_type&& listener) {
      m_listeners.emplace(event, std::move(listener));
    }

    // Returns a temporary handle to await an event. The acceptor function
    // shall return true if it is done waiting and false if it needs to wait
    // more. Ensure that this handle does not outlive the postbox object.
    // Also note that the acceptor function must outlive the await_handle.
    await_handle wait(
      std::string_view event,
      const acceptor_type& acceptor = acceptor_type(accept_always)) {
      std::scoped_lock lock(m_await_mutex);
      m_awaiters.emplace_back(event, acceptor);
      
      auto it = m_awaiters.end();
      it--;
      
      return await_handle(this, it);
    }
    
    // Encapsulates the post-and-wait idiom for sending RPC-style calls
    // to the receiving postbox.
    template <std::predicate<const msgpack::object&> F, class... Args>
    void post_and_wait(std::string_view wait_event, F&& acceptor, std::string_view send_event, Args&&... send_params) {
      auto wait_handle = wait(wait_event, acceptor);
      enqueue(send_event, std::forward<Args>(send_params)...);
      wait_handle.await();
    }

    // Unset the listener for a particular event if it is already set. Do not
    // use while the event loop is active.
    void unlisten(std::string_view event) {
      if (auto it = m_listeners.find(event); it != m_listeners.end()) {
        m_listeners.erase(it);
      }
    }
  };
}  // namespace tc

#endif