/*
This file merely exists to ensure that clangd can determine compile flags
for each header.
*/

#include <tnp/ipc_types.hpp>
#include <tnp/ipc_layout.hpp>
#include <tnp/ipc_proto.hpp>

std::atomic_uint64_t tnp::ipc::Client::request_id = 0;
tnp::ipc::Client::waiter_map_t tnp::ipc::Client::waiter_map;
std::mutex tnp::ipc::Client::waiter_map_mutex;