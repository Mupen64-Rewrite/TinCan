#ifndef TASINPUT2_GUI_GUI_APPLICATION_HPP_INCLUDED
#define TASINPUT2_GUI_GUI_APPLICATION_HPP_INCLUDED

#ifdef _WIN32
#include <wx/msw/winundef.h>
#endif
// wx/app.h includes this but I had to make sure it comes first
#include <wx/init.h>
#include <wx/app.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/utils.h>
#include <memory>
#include "main_window.hpp"

namespace tasinput {
  
  inline const int GUI_SHOW_WINDOW = wxNewId();
  inline const int GUI_CLEANUP = wxNewId();

  class MainApp : public wxApp {
  public:
    MainApp();
    void OnEventLoopEnter(wxEventLoopBase*) override;
    
    void OnShowWindow(wxThreadEvent&);
    void OnCleanup(wxThreadEvent&);

  private:
    wxEventLoopBase* activeEventLoop;
  
    MainWindow* win;
  };
} // tasinput

wxDECLARE_APP(tasinput::MainApp);


#endif //TASINPUT2_GUI_APPLICATION_HPP_INCLUDED
