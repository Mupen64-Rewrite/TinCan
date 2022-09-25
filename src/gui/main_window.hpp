//
// Created by jgcodes on 23/08/22.
//

#ifndef TASINPUT2_GUI_MAIN_WINDOW_HPP_INCLUDED
#define TASINPUT2_GUI_MAIN_WINDOW_HPP_INCLUDED

#include <wx/event.h>
#include <wx/frame.h>
#include <memory>
#include "buttons_panel.hpp"
#include "joystick.hpp"
#include "joystick_panel.hpp"

namespace tasinput {
  
  class MainWindow : public wxFrame {
  public:
    MainWindow(uint32_t idx);
    
    void OnStateUpdated(wxCommandEvent&);
    
    BUTTONS QueryState();
    
    void UpdateVisibleState();
    
  private:
    JoystickPanel* js_panel;
    ButtonsPanel* btn_panel;
    uint32_t ctrl_num;
  };

} // tasinput

#endif //TAS_INPUT_QT_MAIN_WINDOW_HPP_INCLUDED
