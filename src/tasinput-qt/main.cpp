#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <syncstream>
#include <thread>

#include "main_window.hpp"

#define ERR(str) ("ERR:" str)

int main(int argc, char* argv[]) {
  // Ensures that the app stays open
  // as long as the plugin has the pipe open
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);

  auto w = std::make_unique<tnp::MainWindow>();
  
  std::thread ioThread([&]() {
    using std::cin, std::cout, std::ios, std::string;
    using sync = std::osyncstream;
    string x;

    cin.exceptions(ios::badbit | ios::failbit | ios::eofbit);
    while (true) {
      std::getline(cin, x);
      if (x == "show") {
        w->show();
      }
      else if (x == "hide") {
        w->hide();
      }
      else if (x == "query") {
        if (!w->isVisible()) {
          sync(cout) << ERR("Input dialog not visible") << std::endl;
        }
        else {
          BUTTONS res;
          QMetaObject::invokeMethod(
            w.get(), "buttonMask", Qt::BlockingQueuedConnection,
            Q_RETURN_ARG(BUTTONS, res));

          auto flags = cout.flags();
          sync(cout) << std::hex << std::uppercase << std::setw(8)
                    << std::setfill('0') << res.Value << std::endl;
          cout.flags(flags);
        }
      }
      else if (x == "quit") {
        w.reset();
        a.exit(0);
        return;
      }
      else {
        sync(cout) << ERR("Invalid command") << std::endl;
      }
    }
  });
  int res = a.exec();
  ioThread.join();
}
