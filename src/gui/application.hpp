#ifndef TASINPUT2_GUI_GUI_APPLICATION_HPP_INCLUDED
#define TASINPUT2_GUI_GUI_APPLICATION_HPP_INCLUDED

#include <wx/app.h>
#include <memory>
#include "main_window.hpp"

namespace tasinput {

  class MainApp : public wxApp {
  public:
    MainApp() = default;

    bool OnInit() override;

  private:
    MainWindow* win;
  };
} // tasinput


#endif //TASINPUT2_GUI_APPLICATION_HPP_INCLUDED
