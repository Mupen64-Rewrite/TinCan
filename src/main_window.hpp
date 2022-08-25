//
// Created by jgcodes on 23/08/22.
//

#ifndef TASINPUT2_MAIN_WINDOW_HPP_INCLUDED
#define TASINPUT2_MAIN_WINDOW_HPP_INCLUDED

#include <wx/frame.h>
#include <memory>
#include "buttons_panel.hpp"
#include "joystick.hpp"
#include "joystick_panel.hpp"

namespace tasinput {

  class MainWindow : public wxFrame {
  public:
    MainWindow();
    
  private:
    JoystickPanel* jsPanel;
    ButtonsPanel* btnPanel;
  };

} // tasinput

#endif //TAS_INPUT_QT_MAIN_WINDOW_HPP_INCLUDED
