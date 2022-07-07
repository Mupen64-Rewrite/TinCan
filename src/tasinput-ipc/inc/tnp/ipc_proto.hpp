#ifndef TNP_IPC_PROTO_HPP_INCLUDED
#define TNP_IPC_PROTO_HPP_INCLUDED

// Specialized base types for the RPC compiler.
// Follows Google capitalization to fit in with the remainder
// of protobuf.
#include <google/protobuf/message.h>

#include <any>
#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <semaphore>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tnp/ipc_layout.hpp"
#include "tnp_ipc.pb.h"
namespace tnp::ipc {
  class Server {
  public:
    virtual bool handle_request(
      const tnp::ipc::MessageQuery& query, tnp::ipc::MessageReply& reply) const = 0;
  };

  class Client {
  public:
    Client(tnp::ipc_layout::message_queue_t& queue) :
      m_outbound_queue(queue) {}
    static void receive(const tnp::ipc::MessageReply& reply) {
      std::lock_guard l(waiter_map_mutex);
      
      auto& wtr = waiter_map.at(reply.id());
      wtr.reply = reply;
      wtr.sem.release();
    }
  protected:
    tnp::ipc::MessageReply call(
      const google::protobuf::Message& msg, std::string_view service,
      std::string_view method) const {
      tnp::ipc::MessageQuery query;
      query.set_id(request_id++);
      *query.mutable_service() = service;
      *query.mutable_method()  = method;
      query.mutable_data()->PackFrom(msg);
      
      // push query and wait
      waiter_map_t::iterator it;
      {
        std::lock_guard l(waiter_map_mutex);
        m_outbound_queue.get().emplace(query);
        
        it = waiter_map.emplace(
          std::piecewise_construct, 
          std::forward_as_tuple(query.id()), 
          std::forward_as_tuple()).first;
      }
      it->second.sem.acquire();
      tnp::ipc::MessageReply res = *it->second.reply;
      waiter_map.erase(it);
      return res;
    }
  private:
    struct waiter {
      waiter() :
        sem(0), reply(std::nullopt) {}
      
      std::binary_semaphore sem;
      std::optional<tnp::ipc::MessageReply> reply;
    };

    using waiter_map_t = std::unordered_map<uint64_t, waiter>;
    using handler_list_t =
      std::vector<std::function<bool(tnp::ipc::MessageReply)>>;

    static std::atomic_uint64_t request_id;
    static waiter_map_t waiter_map;
    static std::mutex waiter_map_mutex;

    std::reference_wrapper<tnp::ipc_layout::message_queue_t> m_outbound_queue;
  };
  
  class remote_call_error : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };
}  // namespace tnp::ipc

#endif