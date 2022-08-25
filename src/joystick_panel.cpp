#include "joystick_panel.hpp"
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
      
    }
}  // namespace tasinput