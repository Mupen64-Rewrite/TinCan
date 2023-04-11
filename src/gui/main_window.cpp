//
// Created by jgcodes on 23/08/22.
//

#include "main_window.hpp"

#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/toplevel.h>
#include <syncstream>
#include "application.hpp"
#include "buttons_panel.hpp"
#include "joystick.hpp"
#include "joystick_panel.hpp"

namespace tasinput {
  static constexpr long MAIN_WINDOW_STYLE_FLAGS =
    wxSTAY_ON_TOP | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN;

  MainWindow::MainWindow(uint32_t idx) :
    wxFrame(
      nullptr, wxID_ANY, "TinCan", wxDefaultPosition, wxDefaultSize,
      MAIN_WINDOW_STYLE_FLAGS),
    js_panel(new JoystickPanel(this)),
    btn_panel(new ButtonsPanel(this)),
    ctrl_num(idx) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(js_panel, 0, (int) wxALL | wxEXPAND, 3);
    sizer->Add(btn_panel, 0, (int) wxALL | wxEXPAND, 3);
    sizer->AddStretchSpacer();

    SetSizerAndFit(sizer);

    // Events
    Bind(wxEVT_TOGGLEBUTTON, &MainWindow::OnStateUpdated, this);
    Bind(wxEVT_SPINCTRL, &MainWindow::OnStateUpdated, this);
    Bind(TASINPUT_EVT_JSCTRL, &MainWindow::OnStateUpdated, this);
  }

  BUTTONS MainWindow::QueryState() {
    auto js  = js_panel->QueryState().Value;
    auto btn = btn_panel->QueryState().Value;

    return {.Value = js | btn};
  }

  void MainWindow::OnStateUpdated(wxCommandEvent& evt) {
    auto& shm                  = wxGetApp().GetSHM();
    shm.inputs[this->ctrl_num] = QueryState().Value;
  }

  void MainWindow::UpdateVisibleState() {
    using shmflags = ipc::shm_block::shmflags;
    auto& shm      = wxGetApp().GetSHM();

    bool should_show = bool(shm.flags & shmflags::show) &&
      bool(shm.cstate[this->ctrl_num].Present);
    Show(should_show);
  }
}  // namespace tasinput