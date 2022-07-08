#include "shared_objects.hpp"
#include <Qt>
#include <QMetaObject>
#include <QApplication>
#include <stdexcept>
#include "main_window.hpp"
#include "tnp_prtc.pb.h"

namespace tnp {
  delay_ctor<tnp::shm_client> app_shm_client;
  delay_ctor<tnp::prtc::AppServiceServer> app_server;
  std::unordered_map<uint64_t, waiter> waiter_map;
  std::unique_ptr<tnp::MainWindow, qobject_deleter> w;
}  // namespace tnp

namespace tnp::prtc {
  void AppServiceServer::ShowController(
    const tnp::prtc::ShowControllerQuery& query,
    tnp::prtc::ShowControllerReply& reply) const {
    if (query.index() != 0)
      throw std::out_of_range("Controller not plugged in");

    QMetaObject::invokeMethod(
      w.get(), "setVisible", Qt::BlockingQueuedConnection,
      Q_ARG(bool, query.state()));
  }

  void AppServiceServer::QuitApp(
    const tnp::prtc::QuitAppQuery& query,
    tnp::prtc::QuitAppReply& reply) const {
    w.reset();
    qApp->exit();
  }
}  // namespace tnp::prtc