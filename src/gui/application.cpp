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

#define OSS_FMT(content) (static_cast<std::ostringstream&&>(std::ostringstream {} << content).str())

wxIMPLEMENT_APP(tasinput::MainApp);

namespace tasinput {
  MainApp::MainApp() : activeEventLoop(nullptr) {
    SetExitOnFrameDelete(false);
    
    Bind(wxEVT_THREAD, &MainApp::OnShowWindow, this, GUI_SHOW_WINDOW);
    Bind(wxEVT_THREAD, &MainApp::OnCleanup, this, GUI_CLEANUP);
  }
  
  void MainApp::OnEventLoopEnter(wxEventLoopBase* loop) {
    activeEventLoop = wxEventLoop::GetActive();
  }
  
  void MainApp::OnShowWindow(wxThreadEvent& evt) {
    tasinput::DebugLog(M64MSG_STATUS, "TASInput window showing...");
    
    win = new MainWindow;
    
    auto fmt_msg = OSS_FMT("Pointer to active event loop: " << ((void*) activeEventLoop));
    
    DebugLog(M64MSG_INFO, fmt_msg.c_str());
    
    win->Show();
  }
  
  void MainApp::OnCleanup(wxThreadEvent &) {
    ExitMainLoop();
  }
} // tasinput