//
// Created by jgcodes on 23/08/22.
//

#include "application.hpp"
#include <wx/event.h>
#include <wx/evtloop.h>

#include <memory>
#include <sstream>
#include "main_window.hpp"

#include "../global.hpp"

#define OSS_FMT(content) \
  (static_cast<std::ostringstream&&>(std::ostringstream {} << content).str())

wxIMPLEMENT_APP(tasinput::MainApp);

namespace tasinput {
  MainApp::MainApp() {
    SetExitOnFrameDelete(false);
    
    Bind(wxEVT_IDLE, &MainApp::OnIdle, this);
  }

  bool MainApp::OnInit() {
    if (argc < 3) {
      return false;
    }
    
    unsigned long long handle_int, size_int;
    this->argv[1].ToULongLong(&handle_int);
    this->argv[2].ToULongLong(&size_int);

    shm_handle.emplace(
      static_cast<oslib::shm_object::native_handle_type>(handle_int),
      static_cast<size_t>(size_int));
    shm_data.emplace(shm_handle->map());
    
    for (uint32_t i = 0; i < main_wins.size(); i++) {
      main_wins[i] = new MainWindow(i);
    }
    
    prev_flags = 0;
    
    return true;
  }
  
  void MainApp::OnIdle(wxIdleEvent&) {
    using shmflags = ipc::shm_block::shmflags;
    
    uint32_t flags = GetSHM().flags;
    if (flags & shmflags::stop) {
      ExitMainLoop();
    }
    
    if (flags != prev_flags) {
      prev_flags = flags;
      
      for (uint32_t i = 0; i < main_wins.size(); i++) {
        main_wins[i]->UpdateVisibleState();
      }
    }
  }

  ipc::shm_block& MainApp::GetSHM() {
    return shm_data->read<ipc::shm_block>(0x0000);
  }
}  // namespace tasinput