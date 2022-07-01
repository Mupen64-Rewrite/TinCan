#ifndef TNP_RPC_CLIENT_HPP_INCLUDED
#define TNP_RPC_CLIENT_HPP_INCLUDED

#include <google/protobuf/message.h>
#include <string_view>
#include <tnp/ipc_layout.hpp>

namespace tnp::ipc {
  /*?doc?
  Runs a generic RPC call on an RPC area.
  */
  void call_rpc(rpc_area& area, std::string_view service, std::string_view method, const pb::Message& param, pb::Message* const retval);
}
#endif