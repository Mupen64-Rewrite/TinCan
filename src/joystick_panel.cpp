#include "joystick_panel.hpp"
#include <wx/sizer.h>
#include <wx/spinctrl.h>

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
      
      auto* hSizer = new wxBoxSizer(wxHORIZONTAL);
      {
        hSizer->Add(stick, 1, wxSHAPED);
        
        auto* vSizer = new wxBoxSizer(wxVERTICAL);
        {
          auto* spnXSizer = new wxStaticBoxSizer(wxVERTICAL, this, "X");
          spnXSizer->Add(spnX, 1, wxEXPAND | wxALL, 2);
          
          auto* spnYSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Y");
          spnYSizer->Add(spnY, 1, wxEXPAND | wxALL, 2);
          
          vSizer->Add(spnXSizer, 1, wxEXPAND | wxALL, 2);
          vSizer->Add(spnYSizer, 1, wxEXPAND | wxALL, 2);
        }
        hSizer->Add(vSizer);
      }
      
      SetSizerAndFit(hSizer);
    }
    
  
}  // namespace tasinput