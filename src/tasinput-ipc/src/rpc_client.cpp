#include <chrono>
#include <mutex>
#include <stdexcept>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <tnp/rpc_client.hpp>

namespace tnp::ipc {
  void call_rpc(
    rpc_area& area, std::string_view service, std::string_view method,
    const pb::Message& param, pb::Message* const retval) {
    using namespace std::literals;
    // convenient aliases
    auto& cv = area.sgn.cv;
    auto& st = area.sgn.st;
    auto& dt = area.data;
    
    // lock until the RPC area is free
    std::unique_lock<bip::interprocess_mutex> lock(area.sgn.m);
    cv.wait(lock, [&st]() { return st == 0; });
    st = 1;
    
    // Prepare request
    tnp::shmipc::MessageRequest rq;
    *(rq.mutable_method())  = method;
    *(rq.mutable_service()) = service;
    rq.mutable_data()->PackFrom(param);
    if (rq.ByteSizeLong() > sizeof(dt)) {
      throw std::invalid_argument("Message is too long to serialize");
    }
    // Serialize request
    memset(dt.data(), 0, dt.size());
    rq.SerializeToArray(dt.data(), dt.size());
    
    // Notify server and wait
    st = 2;
    cv.notify_one();
    if (!cv.wait_for(lock, 30s, [&st]() { return st == 1; })) {
      // Clean up if server times out
      memset(dt.data(), 0, dt.size());
      st = 0;
      cv.notify_one();
      throw std::logic_error("RPC server timed out");
    }
    
    // Check reply status
    tnp::shmipc::MessageReply rp;
    rp.ParseFromArray(dt.data(), dt.size());
    if (rp.status() != 0) {
      std::ostringstream fmtr;
      fmtr << "RPC returned error " << rp.status();
      throw std::runtime_error(fmtr.str());
    }
    
    // Assign return value
    if (retval != nullptr && rp.has_data()) {
      std::string url = rp.data().type_url();
      url             = url.substr(url.find_last_of('/') + 1);
      if (url != retval->GetDescriptor()->full_name()) {
        throw std::invalid_argument(
          "Return variable's type does not match RPC return type");
      }
    }
    
    // Set state to 0, free lock
    lock.unlock();
    st = 0;
    cv.notify_one();
  }
}  // namespace tnp::ipc