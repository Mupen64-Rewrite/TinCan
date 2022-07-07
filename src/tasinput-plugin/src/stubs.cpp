#include "tnp_prtc.pb.h"

void ::tnp::prtc::AppServiceServer::ShowController(
  const tnp::prtc::ShowControllerQuery&,
  tnp::prtc::ShowControllerReply&) const {}
  
void ::tnp::prtc::AppServiceServer::QuitApp(
  const tnp::prtc::QuitAppQuery&,
  tnp::prtc::QuitAppReply&) const {}