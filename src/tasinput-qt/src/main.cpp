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
#include <thread>
#include <unordered_map>
#include <variant>

#include "config.hpp"
#include "main_window.hpp"
#include "shared_objects.hpp"
#include "tnp_ipc.pb.h"

#include "tnp/ipc_layout.hpp"
#include "tnp_prtc.pb.h"

#define ERR(str) ("ERR:" str)

#define ERR(str) ("ERR:" str)

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 69;
  tnp::app_shm_client.construct(argv[1]);
  tnp::app_server.construct();

  // Ensures that the app stays open
  // as long as the plugin has the pipe open
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  a.setApplicationVersion(TNP_VERSION_STR);
  a.setApplicationDisplayName("TASInput");
  a.setApplicationName("TASInput");
  a.setDesktopFileName("io.github.jgcodes.tasinput-qt");

  tnp::w = decltype(tnp::w)(new tnp::MainWindow());
  

  std::thread postOffice([&]() {
    auto data = tnp::app_shm_client->ipc_data().pull(&tnp::ipc_layout::mq_p2e);
    bool stop_flag = true;
    do {
      std::visit(tnp::overload {
        [&stop_flag](const tnp::ipc::MessageQuery& query) -> void {
          tnp::ipc::MessageReply reply;
          tnp::app_server->handle_request(query, reply);
          tnp::app_shm_client->ipc_data().mq_e2p.emplace(reply);
          
          if (query.method() == "quit" && query.service() == "tnp.prtc.AppService") {
            stop_flag = false;
          }
        },
        [](const tnp::ipc::MessageReply&) -> void {}
      }, data);
    } while (stop_flag);
  });
  int res = a.exec();
  postOffice.join();
}
