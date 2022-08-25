#ifndef TASINPUT2_JOYSTICK_PANEL_HPP
#define TASINPUT2_JOYSTICK_PANEL_HPP

#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <mupen64plus/m64p_plugin.h>
#include "joystick.hpp"

namespace tasinput {
  class JoystickPanel : wxPanel {
  public:
    JoystickPanel(wxWindow* parent);
    
    BUTTONS QueryState();
    
  protected:
    
    void OnSpinXChanged();
    void OnSpinYChanged();
    void OnJoystickChanged();
    
  private:
    Joystick* stick;
    wxSpinCtrl* spnX;
    wxSpinCtrl* spnY;
    
  };
}

#endif