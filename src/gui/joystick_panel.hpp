#ifndef TINCAN_GUI_JOYSTICK_PANEL_HPP
#define TINCAN_GUI_JOYSTICK_PANEL_HPP

#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <mupen64plus/m64p_plugin.h>
#include "joystick.hpp"

namespace tincan {
  class JoystickPanel : public wxPanel {
  public:
    JoystickPanel(wxWindow* parent);
    
    BUTTONS QueryState();
    
  protected:
    
    void OnSpinXChanged(wxSpinEvent&);
    void OnSpinYChanged(wxSpinEvent&);
    void OnJoystickChanged(JoystickControlEvent&);
    
  private:
    Joystick* stick;
    wxSpinCtrl* spnX;
    wxSpinCtrl* spnY;
    
  };
}

#endif