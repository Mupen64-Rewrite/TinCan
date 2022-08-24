//
// Created by jgcodes on 23/08/22.
//

#include "main_window.hpp"
#include <wx/sizer.h>
#include "buttons_panel.hpp"

namespace tasinput {
  MainWindow::MainWindow() :
    wxFrame(nullptr, wxID_ANY, "TASInput"),
    stick(new Joystick(this, wxID_ANY)),
    btnPanel(new ButtonsPanel(this)) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    
    sizer->Add(stick, 1, wxALL | wxEXPAND, 3);
    sizer->Add(btnPanel->GetSizer(), 1, wxALL | wxEXPAND, 3);
    
    SetSizerAndFit(sizer);
  }
} // tasinput