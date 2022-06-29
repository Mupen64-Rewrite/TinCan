#ifndef TNP_MAIN_WINDOW_HPP_
#define TNP_MAIN_WINDOW_HPP_

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
#include "aspect_layout.hpp"
#include "joystick.hpp"
#include "mupen64plus/m64p_plugin.h"

namespace tnp {
  class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    Q_INVOKABLE BUTTONS buttonMask();

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override {
      event->ignore();
    }
  private:
    // XML-based UI seems to not work, so
    // it's manually programmed
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
    
    AspectLayout* jsfSLayout;
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
