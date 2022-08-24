//
// Created by jgcodes on 23/08/22.
//

#include "main_window.hpp"
#include <wx/sizer.h>
#include "buttons_panel.hpp"

namespace tasinput {
  MainWindow::MainWindow() :
    wxFrame(nullptr, wxID_ANY, "TASInput"),
    btnPanel(new ButtonsPanel(this)){
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    
    sizer->Add(btnPanel, 1, wxALL | wxEXPAND, 3);
    
    SetSizerAndFit(sizer);
  }
} // tasinput