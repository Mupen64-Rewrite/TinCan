#ifndef TNP_RPC_SERVER_HPP_INCLUDED
#define TNP_RPC_SERVER_HPP_INCLUDED

#include <atomic>
#include <cstring>
#include <functional>
#include <thread>
#include "shmipc.pb.h"
#include <tnp/ipc_layout.hpp>

namespace tnp::ipc {
  class rpc_server {
  public:
    template <class F>
    rpc_server(rpc_area& area, F&& rq_handler) :
      m_area(&area),
      m_handler(std::forward(rq_handler)),
      m_stop_flag(false),
      m_thread([this]() { server_loop(); }) {}
      
    void stop() {
      // spinlock if there is a request happening
      while (m_area->sgn.st == 2);
      m_stop_flag.test_and_set();
      m_area->sgn.cv.notify_all();
      m_thread.join();
    }

  private:
    void server_loop() {
      // convenient aliases
      auto& cv = m_area->sgn.cv;
      auto& st = m_area->sgn.st;
      auto& dt = m_area->data;
      while (true) {
        // lock until the server gets a request or should stop
        std::unique_lock<bip::interprocess_mutex> lock(m_area->sgn.m);
        cv.wait(lock, [this, &st]() {return m_stop_flag.test() || st == 2;});
        
        // handle the request
        shmipc::MessageRequest rq;
        rq.ParseFromArray(dt.data(), dt.size());
        shmipc::MessageReply rp;
        m_handler(rq, rp);
        
        // send back the reply
        memset(dt.data(), 0, dt.size());
        rp.SerializeToArray(dt.data(), dt.size());
        st = 1;
        cv.notify_one();
      }
    }

    rpc_area* m_area;

    std::function<void(
      const tnp::shmipc::MessageRequest&, tnp::shmipc::MessageReply&)>
      m_handler;
    std::atomic_flag m_stop_flag;

    std::thread m_thread;
  };
}  // namespace tnp::ipc

#endif