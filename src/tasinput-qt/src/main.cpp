#include <qnamespace.h>
#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <syncstream>
#include <thread>

#include "config.hpp"
#include "main_window.hpp"
#include "tnp_ipc.pb.h"

#include "tnp/ipc_layout.hpp"

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
    
    T* operator->() {
      return std::addressof(v);
    }

    template <class... Args>
    void construct(Args... args) {
      new (&v) T(std::forward<Args>(args)...);
    }
  };
  
  delay_ctor<tnp::shm_client> client;
}

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 69;
  client.construct(argv[1]);
  auto data = client->ipc_data().pull(&tnp::ipc_layout::mq_p2e);
  
  std::clog << "Request called method " << std::get<tnp::ipc::MessageRequest>(data).method() << std::endl;
  
  // Ensures that the app stays open
  // as long as the plugin has the pipe open
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  a.setApplicationVersion(TNP_VERSION_STR);
  a.setApplicationDisplayName("TASInput");
  a.setApplicationName("TASInput");
  a.setDesktopFileName("io.github.jgcodes.tasinput-qt");

  std::unique_ptr<tnp::MainWindow, qobject_deleter> w(new tnp::MainWindow());

  std::thread ioThread([&]() {
    using std::cin, std::cout, std::ios, std::string;
    using sync = std::osyncstream;
    string x;

    cin.exceptions(ios::badbit | ios::failbit | ios::eofbit);
    while (true) {
      std::getline(cin, x);
      if (x == "show") {
        QMetaObject::invokeMethod(
          w.get(), "show", Qt::BlockingQueuedConnection);
        sync(cout) << "DONE" << std::endl;
      }
      else if (x == "hide") {
        QMetaObject::invokeMethod(
          w.get(), "hide", Qt::BlockingQueuedConnection);
        sync(cout) << "DONE" << std::endl;
      }
      else if (x == "query") {
        if (!w->isVisible()) {
          sync(cout) << ERR("Input dialog not visible") << std::endl;
        }
        else {
          BUTTONS res;
          QMetaObject::invokeMethod(
            w.get(), "buttonMask", Qt::BlockingQueuedConnection,
            Q_RETURN_ARG(BUTTONS, res));

          auto flags = cout.flags();
          sync(cout) << std::hex << std::uppercase << std::setw(8)
                     << std::setfill('0') << res.Value << std::endl;
          cout.flags(flags);
        }
      }
      else if (x == "quit") {
        QMetaObject::invokeMethod(
          w.get(), "hide", Qt::BlockingQueuedConnection);
        w->deleteLater();
        a.quit();
        sync(cout) << "DONE" << std::endl;
        return;
      }
      else {
        sync(cout) << ERR("Invalid command") << std::endl;
      }
    }
  });
  int res = a.exec();
  ioThread.join();
}
