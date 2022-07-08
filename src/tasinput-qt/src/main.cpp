#include <fmt/core.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <QApplication>
#include <QDirIterator>
#include <QIcon>
#include <QMainWindow>
#include <QMetaObject>
#include <QStandardPaths>
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

void set_icon_theme() {
  if (qApp->platformName() == "wayland") {
    // Wayland is special. Copy the icon theme into the theme folders.
    QDir local_share =
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
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
  else {
    QIcon icon(":res/share/icons/hicolor/128x128/apps/io.github.jgcodes.tasinput-qt.png");
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

  // Ensures that the app stays open
  // as long as the plugin has the pipe open
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  a.setApplicationVersion(TNP_VERSION_STR);
  a.setApplicationDisplayName("TASInput");
  a.setApplicationName("TASInput");
  a.setDesktopFileName("io.github.jgcodes.tasinput-qt");

  tnp::w = decltype(tnp::w)(new tnp::MainWindow());
  set_icon_theme();

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

            if (
              query.method() == "QuitApp" &&
              query.service() == "tnp.prtc.AppService") {
              stop_flag = false;
            }
          },
          [](const tnp::ipc::MessageReply&) -> void {
          }},
        data);

      if (!stop_flag && tnp::app_shm_client->ipc_data().mq_p2e.empty())
        break;
    }
  });
  int res = a.exec();
  postOffice.join();
}
