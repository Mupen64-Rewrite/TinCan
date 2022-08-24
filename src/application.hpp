//
// Created by jgcodes on 23/08/22.
//

#ifndef TASINPUT2_APPLICATION_HPP_INCLUDED
#define TASINPUT2_APPLICATION_HPP_INCLUDED

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


#endif //TASINPUT2_APPLICATION_HPP_INCLUDED