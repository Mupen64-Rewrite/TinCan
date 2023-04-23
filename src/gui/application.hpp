#ifndef TINCAN_GUI_GUI_APPLICATION_HPP_INCLUDED
#define TINCAN_GUI_GUI_APPLICATION_HPP_INCLUDED

#include <thread>
#ifdef _WIN32
  #include <wx/msw/winundef.h>
#endif
// wx/app.h includes this but I had to make sure it comes first
#include <wx/app.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/init.h>
#include <wx/timer.h>
#include <wx/utils.h>
#include <memory>
#include <chrono>
#include <optional>
#include "../oslib/shmem.hpp"
#include "../ipc/shm_block.hpp"
#include <zmq.hpp>
#include "main_window.hpp"

namespace tincan {

  class MainApp : public wxApp {
  public:
    MainApp();
    bool OnInit() override;
    
    void OnIdle(wxIdleEvent&);
    
    void OnSyncTimer(wxTimerEvent&);
    
    void OnUnhandledException() override;
  
    ipc::shm_block& GetSHM();
  private:
    std::array<MainWindow*, 4> main_wins;
    uint32_t prev_flags;
    std::chrono::system_clock::time_point prev_idle;
    
    zmq::context_t zmq_context;
    zmq::socket_t zmq_socket;
  };
}  // namespace tincan

wxDECLARE_APP(tincan::MainApp);

#endif  // TASINPUT2_GUI_APPLICATION_HPP_INCLUDED
