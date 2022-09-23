#ifndef TASINPUT2_GUI_GUI_APPLICATION_HPP_INCLUDED
#define TASINPUT2_GUI_GUI_APPLICATION_HPP_INCLUDED

#include <thread>
#ifdef _WIN32
  #include <wx/msw/winundef.h>
#endif
// wx/app.h includes this but I had to make sure it comes first
#include <wx/app.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/init.h>
#include <wx/utils.h>
#include <memory>
#include <optional>
#include "../oslib/shmem.hpp"
#include "../ipc/shm_block.hpp"
#include "main_window.hpp"

namespace tasinput {

  class MainApp : public wxApp {
  public:
    MainApp();
    bool OnInit() override;
  
    ipc::shm_block& GetSHM();
  private:
    std::array<MainWindow*, 4> main_wins;

    std::thread shm_thread;
    std::optional<oslib::shm_object> shm_handle;
    std::optional<oslib::shm_mapping> shm_data;
  };
}  // namespace tasinput

wxDECLARE_APP(tasinput::MainApp);

#endif  // TASINPUT2_GUI_APPLICATION_HPP_INCLUDED
