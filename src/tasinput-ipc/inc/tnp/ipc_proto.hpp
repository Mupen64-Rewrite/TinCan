#ifndef TNP_IPC_PROTO_HPP_INCLUDED
#define TNP_IPC_PROTO_HPP_INCLUDED

// Specialized base types for the RPC compiler.
// Follows Google capitalization to fit in with the remainder
// of protobuf.
#include "tnp_ipc.pb.h"
namespace tnp::ipc {
  class Server {
  protected:
    virtual bool handle_request(
      const tnp::ipc::MessageQuery& query, tnp::ipc::MessageReply& reply);
  };
}  // namespace tnp::ipc

#endif