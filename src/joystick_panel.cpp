#include "joystick_panel.hpp"
#include <wx/sizer.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>

#include <iostream>
#include "joystick.hpp"

namespace tasinput {
  JoystickPanel::JoystickPanel(wxWindow* parent) :
    wxPanel(parent, wxID_ANY),
    stick(new Joystick(this, wxID_ANY)),
    spnX(new wxSpinCtrl(
      this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
      -128, 127, 0)),
    spnY(new wxSpinCtrl(
      this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
      -128, 127, 0)) {
    stick->SetMinSize({128, 128});

    auto* hSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Joystick");
    {
      hSizer->Add(stick, 1, wxSHAPED | wxALIGN_CENTER | wxALL, 2);

      auto* vSizer = new wxBoxSizer(wxVERTICAL);
      {
        auto* spnXSizer = new wxStaticBoxSizer(wxVERTICAL, this, "X");
        spnXSizer->Add(spnX, 1, wxEXPAND | wxALL, 2);

        auto* spnYSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Y");
        spnYSizer->Add(spnY, 1, wxEXPAND | wxALL, 2);

        vSizer->Add(spnXSizer, 1, wxEXPAND | wxALL, 2);
        vSizer->Add(spnYSizer, 1, wxEXPAND | wxALL, 2);
      }
      hSizer->Add(vSizer, 0, wxALIGN_CENTER_VERTICAL);
    }

    SetSizerAndFit(hSizer);
    
    spnX->Bind(wxEVT_SPINCTRL, &JoystickPanel::OnSpinXChanged, this);
    spnY->Bind(wxEVT_SPINCTRL, &JoystickPanel::OnSpinYChanged, this);
    stick->Bind(TASINPUT_EVT_JSCTRL, &JoystickPanel::OnJoystickChanged, this);
  }
  
  BUTTONS JoystickPanel::QueryState() {
    return stick->QueryState();
  }
  
  void JoystickPanel::OnJoystickChanged(JoystickControlEvent& evt) {
    auto pos = evt.GetValue();
    spnX->SetValue(pos.x);
    spnY->SetValue(pos.y);
  }
  void JoystickPanel::OnSpinXChanged(wxSpinEvent& evt) {
    auto val = evt.GetValue();
    stick->SetPosX(val);
  }
  void JoystickPanel::OnSpinYChanged(wxSpinEvent& evt) {
    auto val = evt.GetValue();
    stick->SetPosY(val);
  }
}  // namespace tasinput