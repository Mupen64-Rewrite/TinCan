#ifndef TNP_SHARED_OBJECTS_HPP
#define TNP_SHARED_OBJECTS_HPP

#include "tnp_prtc.pb.h"
#include <tnp/ipc_layout.hpp>
#include <QObject>

#include <any>
#include <concepts>
#include <semaphore>
#include <unordered_map>
#include <utility>

#include "main_window.hpp"

namespace tnp {
  struct qobject_deleter {
  template <std::derived_from<QObject> T>
    void operator()(T* obj) {
      obj->deleteLater();
    }
  };
  
  template <class T>
  union delay_ctor {
  private:
    char _dummy_please_ignore;

  public:
    T v;

    delay_ctor() : _dummy_please_ignore(0) {}
    ~delay_ctor() { v.~T(); }

    operator T&() { return v; }

    T* operator->() { return std::addressof(v); }

    template <class... Args>
    void construct(Args&&... args) {
      new (&v) T(std::forward<Args>(args)...);
    }
  };
  
  template <class... Fs>
  struct overload : Fs... {
    using Fs::operator()...;
  };
  template <class... Fs>
  overload(Fs&&...) -> overload<Fs...>;

  struct waiter {
    std::binary_semaphore sem;
    std::any data;
  };

  extern delay_ctor<tnp::shm_client> app_shm_client;
  extern delay_ctor<tnp::prtc::AppServiceServer> app_server;
  extern std::unordered_map<uint64_t, waiter> waiter_map;
  extern std::unique_ptr<tnp::MainWindow, qobject_deleter> w;
}  // namespace tnp
#endif