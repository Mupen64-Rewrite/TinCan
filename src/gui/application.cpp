//
// Created by jgcodes on 23/08/22.
//

#include "application.hpp"
#include <wx/event.h>

#include <memory>
#include "main_window.hpp"

#include "../global.hpp"

wxIMPLEMENT_APP_NO_MAIN(tasinput::MainApp);

namespace tasinput {
  MainApp::MainApp() {
    SetExitOnFrameDelete(false);
    
    Bind(wxEVT_THREAD, &MainApp::OnShowWindow, this, GUI_SHOW_WINDOW);
    Bind(wxEVT_THREAD, &MainApp::OnCleanup, this, GUI_CLEANUP);
  }
  
  bool MainApp::OnInit() {
    return true;
  }
  
  void MainApp::OnShowWindow(wxThreadEvent& evt) {
    tasinput::DebugLog(M64MSG_STATUS, "TASInput window showing...");
    
    win = new MainWindow;
    win->Show();
  }
  
  void MainApp::OnCleanup(wxThreadEvent &) {
    ExitMainLoop();
  }
} // tasinput