#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <Qt>

#include <any>
#include <iomanip>
#include <iostream>
#include <memory>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <syncstream>
#include <thread>
#include <unordered_map>
#include <variant>

#include "config.hpp"
#include "main_window.hpp"
#include "tnp_ipc.pb.h"

#include "tnp/ipc_layout.hpp"
#include "tnp_prtc.pb.h"

#define ERR(str) ("ERR:" str)

#define ERR(str) ("ERR:" str)

struct qobject_deleter {
  template <std::derived_from<QObject> T>
  void operator()(T* obj) {
    obj->deleteLater();
  }
};

namespace {
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
    void construct(Args... args) {
      new (&v) T(std::forward<Args>(args)...);
    }
  };

  struct waiter {
    std::binary_semaphore sem;
    std::any data;
  };

  delay_ctor<tnp::shm_client> client;
  std::unordered_map<uint64_t, waiter> waiter_map;
}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 69;
  client.construct(argv[1]);
  auto data = client->ipc_data().pull(&tnp::ipc_layout::mq_p2e);

  std::clog << "Request called method "
            << std::get<tnp::ipc::MessageRequest>(data).method() << std::endl;

  // Ensures that the app stays open
  // as long as the plugin has the pipe open
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  a.setApplicationVersion(TNP_VERSION_STR);
  a.setApplicationDisplayName("TASInput");
  a.setApplicationName("TASInput");
  a.setDesktopFileName("io.github.jgcodes.tasinput-qt");

  std::unique_ptr<tnp::MainWindow, qobject_deleter> w(new tnp::MainWindow());

  std::thread postOffice([&]() {
    while (true) {
      auto obj = client->ipc_data().pull(&tnp::ipc_layout::mq_p2e);
      if (obj.index() == 0) {
        // the object is a request
        auto rq = std::get<0>(obj);
        // only handle methods in our service (not really necessary)
        if (rq.service() != "io.github.jgcodes.tasinput-qt")
          continue;

        std::string_view mt = *(rq.mutable_method());
        // determine and handle method
        if (mt == "show_controller") {
          do {
            tnp::prtc::ShowControllerQuery q;
            rq.data().UnpackTo(&q);

            if (q.index() != 0) {
              auto exc = std::out_of_range("Only controller 1 supported ATM");
              client->ipc_data().push_exception(
                &tnp::ipc_layout::mq_e2p, exc, rq.id());
              break;
            }

            w->setVisible(q.state());

            tnp::prtc::ShowControllerReply r;
            client->ipc_data().push_reply(&tnp::ipc_layout::mq_e2p, r, rq.id());
          } while (false);
        }
        else if (mt == "quit") {
          do {
            QMetaObject::invokeMethod(
              w.get(), "hide", Qt::BlockingQueuedConnection);
            w->deleteLater();
            a.quit();
            tnp::prtc::QuitAppReply r;
            client->ipc_data().push_reply(&tnp::ipc_layout::mq_e2p, r, rq.id());
            return;
          } while (false);
        }
        else {
          auto exc = std::invalid_argument("Invalid method call");
          client->ipc_data().push_exception(
            &tnp::ipc_layout::mq_e2p, exc, rq.id());
        }
      }
      else {
        // the object is a reply
        auto re = std::get<1>(obj);
        if (re.error()) {
          tnp::ipc::Exception exception;
          re.data().UnpackTo(&exception);
          
          std::ostringstream oss;
          oss << exception.type() << " thrown from RPC: ";
          oss << exception.detail();
          throw std::runtime_error(oss.str());
        }
        // If exists, store a result, then release its semaphore
        auto& waiter = waiter_map.at(re.id());
        if (re.data().Is<tnp::prtc::GetConfigKeyReply>()) {
          tnp::prtc::GetConfigKeyReply r;
          re.data().UnpackTo(&r);
          std::variant<std::string, uint32_t, float, bool> value;
          switch (r.value_case()) {
            using tnp::prtc::GetConfigKeyReply;
            case GetConfigKeyReply::kStrValue: {
              value = r.str_value();
            } break;
            case GetConfigKeyReply::kIntValue: {
              value = r.int_value();
            } break;
            case GetConfigKeyReply::kFloatValue: {
              value = r.float_value();
            } break;
            case GetConfigKeyReply::kBoolValue: {
              value = r.bool_value();
            } break;
            default: {
              throw std::runtime_error("GetKeyConfig didn't return a value");
            } break;
          }
          
          waiter.data = value;
        }
        
        waiter.sem.release();
      }
    }
  });
  int res = a.exec();
  postOffice.join();
}
