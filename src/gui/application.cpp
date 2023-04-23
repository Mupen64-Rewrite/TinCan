//
// Created by jgcodes on 23/08/22.
//

#include "application.hpp"
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/timer.h>

#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <syncstream>
#include <thread>
#include <typeinfo>
#include "main_window.hpp"
#include <zmq.hpp>


#ifdef __GNUG__
  #include <cxxabi.h>
  #include <cstdlib>
  #include <memory>
#endif

namespace {
#ifdef __GNUG__
  std::string pretty_name(const std::type_info& t) {
    int status = -4;

    std::unique_ptr<char, void (*)(void*)> abi_string(
      abi::__cxa_demangle(t.name(), nullptr, nullptr, &status), std::free);

    return (status == 0) ? abi_string.get() : t.name();
  }
#else
  std::string pretty_name(std::type_info& t) {
    return t.name();
  }
#endif
  void print_exception(const std::exception& exc, size_t depth = 0) {
    {
      std::osyncstream out(std::cerr);
      if (depth == 0)
        out << "Unhandled exception!\n";

      out << "#" << std::setw(2) << depth << " " << pretty_name(typeid(exc));
      out << ": \n  " << exc.what();
    }
    
    try {
      std::rethrow_if_nested(exc);
    }
    catch (const std::exception& exc) {
      print_exception(exc, depth + 1);
    }
  }
  
  const auto SYNC_TIMER_ID = wxNewId();
}  // namespace

namespace tincan {
  MainApp::MainApp() {
    SetExitOnFrameDelete(false);

    Bind(wxEVT_IDLE, &MainApp::OnIdle, this);
  }

  bool MainApp::OnInit() {
    try {
      if (argc < 3) {
        return false;
      }

      unsigned long long handle_int, size_int;
      this->argv[1].ToULongLong(&handle_int);
      this->argv[2].ToULongLong(&size_int);
      
      MainWindow* mw = new MainWindow(0);
      mw->Show();
    }
    catch (const std::exception& e) {
      print_exception(e);
      return false;
    }

    return true;
  }

  void MainApp::OnIdle(wxIdleEvent& evt) {
  }

  void MainApp::OnUnhandledException() {
    try {
      throw;
    }
    catch (const std::exception& e) {
      print_exception(e);
    }
    catch (...) {
      std::cerr << "Unknown exception thrown\n";
    }
  }
}  // namespace tincan

wxIMPLEMENT_APP(tincan::MainApp);