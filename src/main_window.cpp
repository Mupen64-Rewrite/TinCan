//
// Created by jgcodes on 23/08/22.
//

#include "main_window.hpp"
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/toplevel.h>
#include "buttons_panel.hpp"
#include "joystick_panel.hpp"

namespace tasinput {
  static constexpr long MAIN_WINDOW_STYLE_FLAGS = 
    wxDEFAULT_FRAME_STYLE;
  
  MainWindow::MainWindow() :
    wxFrame(nullptr, wxID_ANY, "TASInput", wxDefaultPosition, wxDefaultSize, MAIN_WINDOW_STYLE_FLAGS),
    jsPanel(new JoystickPanel(this)),
    btnPanel(new ButtonsPanel(this)) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    
    sizer->Add(jsPanel, 1, wxALL | wxEXPAND, 3);
    sizer->Add(btnPanel, 1, wxALL | wxEXPAND, 3);
    
    SetSizerAndFit(sizer);
  }
} // tasinput