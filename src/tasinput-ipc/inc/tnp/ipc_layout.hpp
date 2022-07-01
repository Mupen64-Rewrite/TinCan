#ifndef TNP_SHM_IPC_LAYOUT_HPP_INCLUDED
#define TNP_SHM_IPC_LAYOUT_HPP_INCLUDED

#include <array>
#include <atomic>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <google/protobuf/message.h>

#include "shmipc.pb.h"
#include "tnp/wrapper.hpp"

namespace tnp::ipc {
  namespace bip = boost::interprocess;
  namespace pb  = google::protobuf;

  // State that is constantly updated
  struct ipc_shared_data {
    std::array<std::atomic_uint32_t, 4> ctrl_buttons;
  };

  struct rpc_area {
    static constexpr size_t size = 2048;

    struct {
      bip::interprocess_mutex m;
      bip::interprocess_condition cv;
      // 0 = ready
      // 1 = held by client
      // 2 = held by server
      std::atomic_uint8_t st = 0;
    } sgn;
    std::array<char, size - sizeof(sgn)> data;

    void call(
      std::string_view method, std::string_view service,
      const pb::Message& params, pb::Message* retval = nullptr) {
      std::unique_lock lk(sgn.m);
      sgn.cv.wait(lk, [&]() { return sgn.st == 0; });
      sgn.st = 1;

      tnp::shmipc::MessageRequest rq;
      *(rq.mutable_method())  = method;
      *(rq.mutable_service()) = service;
      rq.mutable_data()->PackFrom(params);
      if (rq.ByteSizeLong() > sizeof(data)) {
        throw std::invalid_argument("Message is too long to serialize");
      }
      // clear array, then send
      memset(data.data(), 0, data.size());
      rq.SerializeToArray(data.data(), data.size());

      sgn.st = 2;
      sgn.cv.notify_one();

      sgn.cv.wait(lk, [&]() { return sgn.st == 1; });

      tnp::shmipc::MessageReply rp;
      rp.ParseFromArray(data.data(), data.size());
      if (rp.status() != 0) {
        std::ostringstream fmtr;
        fmtr << "RPC returned error " << rp.status();
        throw std::runtime_error(fmtr.str());
      }

      if (retval != nullptr && rp.has_data()) {
        std::string url = rp.data().type_url();
        url             = url.substr(url.find_last_of('/') + 1);
        if (url != retval->GetDescriptor()->full_name()) {
          throw std::logic_error(
            "Return variable's type does not match RPC return type");
        }
      }

      sgn.st = 0;
      sgn.cv.notify_one();
    }
    void server_loop(
      const std::invocable<
        const tnp::shmipc::MessageRequest&, tnp::shmipc::MessageReply&> auto&
        handler,
      std::atomic_flag& stop_flag) {
      std::unique_lock lk(sgn.m);
      bool flag = false;
      while (true) {
        sgn.cv.wait(lk, [&]() { return (flag = stop_flag.test()) || sgn.st == 2; });
        if (flag)
          break;

        tnp::shmipc::MessageRequest rq;
        rq.ParseFromArray(data.data(), data.size());

        tnp::shmipc::MessageReply rp;
        try {
          handler(rq, rp);
        }
        catch (...) {
          rp.set_status(1);
          rp.clear_data();
        }

        sgn.st = 1;
        sgn.cv.notify_one();
      }
    }
  };
  static_assert(
    sizeof(rpc_area) == rpc_area::size,
    "This system seems pretty unconventional");

  class ipc_layout {
  public:
    ipc_layout(bool is_server) :
      shm_obj(shm_size, "io.github.jgcodes.tasinput-qt", is_server),
      region(shm_obj.map()) {
      // Initialize members
      if (is_server) {
        new (rel_ptr(0x0000)) ipc_shared_data();
        new (rel_ptr(0x1000)) rpc_area();
        new (rel_ptr(0x1800)) rpc_area();
      }
    }

    void* rel_ptr(uintptr_t offset) {
      if (offset > region.get_size())
        throw std::out_of_range("Address lies outside of SHM page");
      return static_cast<char*>(region.get_address()) + offset;
    }

    ipc_shared_data& shared_data() {
      return *static_cast<ipc_shared_data*>(rel_ptr(0));
    }
    rpc_area& p2a_rpc() { return *static_cast<rpc_area*>(rel_ptr(0x1000)); }
    rpc_area& a2p_rpc() { return *static_cast<rpc_area*>(rel_ptr(0x1800)); }

  private:
    // I used 8K instead of something like 6K, because Windows uses 4K aligned
    // sizes and I wanted 2K for each RPC space
    static constexpr size_t shm_size = 8192;

    shm_wrapper shm_obj;
    bip::mapped_region region;
  };
}  // namespace tnp

#endif