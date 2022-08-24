//
// Created by jgcodes on 23/08/22.
//

#include "application.hpp"

#include <memory>

wxIMPLEMENT_APP_NO_MAIN(tasinput::MainApp);

namespace tasinput {
  bool MainApp::OnInit() {
    win = new MainWindow;
    win->Show();

    return true;
  }
} // tasinput