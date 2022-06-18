#ifndef TNP_MAIN_WINDOW_HPP_
#define TNP_MAIN_WINDOW_HPP_

#include <qboxlayout.h>
#include <qtmetamacros.h>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QResizeEvent>
#include <QWidget>

#include <iostream>
#include "joystick.hpp"
#include "mupen64plus/m64p_plugin.h"
#include "titlebar.hpp"

namespace tnp {
  class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    Q_INVOKABLE BUTTONS buttonMask();

  protected:
    void resizeEvent(QResizeEvent* event) override {
      const auto& size = event->size();
      //std::cout << "New size: " << size.width() << "x" << size.height() << '\n';
    }

  private:
    // XML-based UI seems to not work, so
    // it's manually programmed
    QVBoxLayout* baseLayout;
    TitleBar* titleBar;
    
    QVBoxLayout* rootLayout;

    // Button frame
    // ============
    QGroupBox* btnFrame;
    QGridLayout* btnfLayout;

    QPushButton* btnfButtonL;
    QPushButton* btnfButtonZ;
    QPushButton* btnfButtonR;

    QPushButton* btnfButtonA;
    QPushButton* btnfButtonB;
    QPushButton* btnfButtonS;
    QWidget* btnfSpacerR3;

    QLabel* labelD;
    QPushButton* btnfButtonDU;
    QPushButton* btnfButtonDD;
    QPushButton* btnfButtonDL;
    QPushButton* btnfButtonDR;

    QLabel* labelC;
    QPushButton* btnfButtonCU;
    QPushButton* btnfButtonCD;
    QPushButton* btnfButtonCL;
    QPushButton* btnfButtonCR;
    
    // Joystick frame
    // ==============
    QGroupBox* jsFrame;
    QGridLayout* jsfLayout;
    
    Joystick* jsfStick;
    
    QGroupBox* jsfGroupX;
    QVBoxLayout* jsfGXLayout;
    QSpinBox* jsfSpinX;
    
    QGroupBox* jsfGroupY;
    QVBoxLayout* jsfGYLayout;
    QSpinBox* jsfSpinY;

    void setupRootLayout();
    void setupButtonFrame();
    void setupJoystickFrame();
  };
}  // namespace tnp
#endif
