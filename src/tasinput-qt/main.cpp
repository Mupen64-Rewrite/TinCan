#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <memory>
#include <string>
#include <syncstream>
#include <thread>
#include <iomanip>
#include <iostream>

#include "main_window.hpp"



int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  auto w = std::make_unique<tnp::MainWindow>();
  w->show();
  std::thread ioThread([&]() {
    using std::cin, std::cout, std::ios, std::string;
    using sync = std::osyncstream;
    string x;
    
    cin.exceptions(ios::badbit | ios::failbit);
    while (!std::getline(cin, x).eof()) {
      if (x == "query") {
        BUTTONS res;
        QMetaObject::invokeMethod(w.get(), "buttonMask", Qt::BlockingQueuedConnection, Q_RETURN_ARG(BUTTONS, res));
        
        auto flags = cout.flags();
        sync(cout) << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
          << res.Value << std::endl;
        cout.flags(flags);
      }
      else if (x == "quit") {
        w.reset();
        break;
      }
      else {
        sync(cout) << "ERR_INVALID_QUERY" << std::endl;
      }
    }
    w.reset();
  });
  int res = a.exec();
  ioThread.join();
}
