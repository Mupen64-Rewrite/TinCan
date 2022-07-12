#include <fmt/core.h>
#include <sched.h>
#include <unistd.h>
#include <QApplication>
#include <QDirIterator>
#include <QIcon>
#include <QMainWindow>
#include <QMetaObject>
#include <QStandardPaths>
#include <Qt>

#include <any>
#include <atomic>
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

#if defined(__linux__) || defined(__APPLE__)
  #include <signal.h>
  #include <unistd.h>
#endif

#define ERR(str) ("ERR:" str)

void set_icon_theme() {
  if (qApp->platformName() == "wayland") {
    QDir local_share =
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (!QFile::exists(
          local_share.path() + QDir::separator() +
          "applications/io.github.jgcodes.tasinput-qt.desktop")) {
      // Wayland is special because it demands us to drop our icons
      // in the XDG theming folders and provide a .desktop.
      // Luckily I have this covered.

      QDir res_share(":/res/share");

      QDirIterator qdi(res_share, QDirIterator::Subdirectories);
      while (qdi.hasNext()) {
        QFileInfo f = qdi.nextFileInfo();

        if (f.isDir()) {
          local_share.mkpath(res_share.relativeFilePath(f.filePath()));
        }
        else if (f.isFile()) {
          QString dst = local_share.path() + QDir::separator() +
            res_share.relativeFilePath(f.filePath());
          QFile::copy(f.filePath(), dst);
          QFile::setPermissions(dst, QFile::Permissions(0x6644));
        }
      }
    }
  }
  else {
    QIcon icon(
      ":res/share/icons/hicolor/128x128/apps/io.github.jgcodes.tasinput-qt.png");
    for (auto size : {16, 32, 38, 64, 128}) {
      QString path =
        QStringLiteral(
          ":res/share/icons/hicolor/%1x%1/apps/io.github.jgcodes.tasinput-qt.png")
          .arg(QString::number(size));

      icon.addFile(path, QSize(size, size));
    }
    tnp::w->setWindowIcon(icon);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 69;
  tnp::app_shm_client.construct(argv[1]);
  tnp::app_server.construct();

  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  a.setApplicationVersion(TNP_VERSION_STR);
  a.setApplicationDisplayName("TASInput");
  a.setApplicationName("TASInput");
  a.setDesktopFileName("io.github.jgcodes.tasinput-qt");

  tnp::w = decltype(tnp::w)(new tnp::MainWindow());
  set_icon_theme();

  // Ok, a post office was my best analogy.
  // It sorts out requests and replies and deals with them accordingly.
  std::thread postOffice([&]() {
    bool stop_flag = true;
    while (true) {
      auto data =
        tnp::app_shm_client->ipc_data().pull(&tnp::ipc_layout::mq_p2e);
      std::visit(
        tnp::overload {
          [&stop_flag](const tnp::ipc::MessageQuery& query) -> void {
            tnp::ipc::MessageReply reply;
            tnp::app_server->handle_request(query, reply);
            tnp::app_shm_client->ipc_data().mq_e2p.emplace(reply);

            // Since the generated server class can't do this, we'll have to
            // do this ourselves.
            if (
              query.method() == "QuitApp" &&
              query.service() == "tnp.prtc.AppService") {
              stop_flag = false;
            }
          },
          [](const tnp::ipc::MessageReply&) -> void {
            // If message calls sent the other way happen.
          }},
        data);

      if (!stop_flag && tnp::app_shm_client->ipc_data().mq_p2e.empty())
        break;
    }
  });
#if defined(__linux__) || defined(__APPLE__)
  // Automatic death thread: kills this process
  // if it has been reparented
  std::atomic_bool pid_watch_stop_flag {true};
  std::thread killer([&]() {
    pid_t prev_ppid = std::stoul(argv[2]);
    while (pid_watch_stop_flag) {
      if (getppid() != prev_ppid)
        kill(getpid(), SIGTERM);
      usleep(50000);
    }
  });
#endif
  int res = a.exec();
  postOffice.join();

#if defined(__linux__) || defined(__APPLE__)
  pid_watch_stop_flag = false;
  killer.join();
#endif
}
