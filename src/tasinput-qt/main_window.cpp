#include "main_window.hpp"
#include <qlabel.h>
#include <qnamespace.h>
#include <qstringliteral.h>
#include <QBoxLayout>
#include <QWindow>
#include <QGridLayout>
#include <QGroupBox>
#include <QLayoutItem>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStatusBar>
#include <QWidget>
#include "aspect_layout.hpp"
#include "joystick.hpp"
#include "mupen64plus/m64p_plugin.h"

#define with(var) if (var; true)

namespace {
  const QString BUTTON_STYLING = R"!css!(
    min-width: 1.3em;
  )!css!";
}

namespace tnp {
  MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("TASInput");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    auto minSize = minimumSizeHint();
    resize(minSize);
    setMinimumSize(minSize);
    setMaximumSize(minSize);

    setSizePolicy(QSizePolicy {QSizePolicy::Fixed, QSizePolicy::Fixed});
    setStatusBar(nullptr);

    setupRootLayout();
    setupJoystickFrame();
    setupButtonFrame();
  }

  BUTTONS MainWindow::buttonMask() {
    BUTTONS res;
    res.X_AXIS = jsfStick->xPos();
    res.Y_AXIS = jsfStick->yPos();

    res.L_TRIG = btnfButtonL->isChecked();
    res.Z_TRIG = btnfButtonZ->isChecked();
    res.R_TRIG = btnfButtonR->isChecked();

    res.U_DPAD = btnfButtonDU->isChecked();
    res.D_DPAD = btnfButtonDD->isChecked();
    res.L_DPAD = btnfButtonDL->isChecked();
    res.R_DPAD = btnfButtonDR->isChecked();

    res.U_CBUTTON = btnfButtonCU->isChecked();
    res.D_CBUTTON = btnfButtonCD->isChecked();
    res.L_CBUTTON = btnfButtonCL->isChecked();
    res.R_CBUTTON = btnfButtonCR->isChecked();

    res.START_BUTTON = btnfButtonS->isChecked();
    res.B_BUTTON     = btnfButtonB->isChecked();
    res.A_BUTTON     = btnfButtonA->isChecked();
    return res;
  }
  
  void MainWindow::mousePressEvent(QMouseEvent* event) {
    switch (event->button()) {
      case Qt::LeftButton: {
        windowHandle()->startSystemMove();
      }
      default: break;
    }
  }

  void MainWindow::setupRootLayout() {
    QWidget* root = new QWidget(this);
    rootLayout    = new QVBoxLayout(root);
    rootLayout->setContentsMargins(8, 8, 8, 8);

    setCentralWidget(root);
  }

  void MainWindow::setupJoystickFrame() {
    jsFrame   = new QGroupBox(this);
    jsfLayout = new QGridLayout(jsFrame);

    jsFrame->setTitle("Joystick");
    {
      jsfStick = new Joystick(this);
      jsfSLayout = new AspectLayout(nullptr, 1);
      jsfSLayout->addWidget(jsfStick);
      jsfLayout->addLayout(jsfSLayout, 0, 0, 2, 1);

      jsfGroupX   = new QGroupBox(this);
      jsfGXLayout = new QVBoxLayout(jsfGroupX);
      jsfGroupX->setTitle("X");
      {
        jsfSpinX = new QSpinBox(this);
        jsfSpinX->setRange(-128, 127);
        jsfGXLayout->addWidget(jsfSpinX);
      }
      jsfLayout->addWidget(jsfGroupX, 0, 1);

      jsfGroupY   = new QGroupBox(this);
      jsfGYLayout = new QVBoxLayout(jsfGroupY);
      jsfGroupY->setTitle("Y");
      {
        jsfSpinY = new QSpinBox(this);
        jsfSpinY->setRange(-128, 127);
        jsfGYLayout->addWidget(jsfSpinY);
      }
      jsfLayout->addWidget(jsfGroupY, 1, 1);

      QObject::connect(
        jsfSpinX, &QSpinBox::valueChanged, jsfStick, &Joystick::setXPos);
      QObject::connect(
        jsfStick, &Joystick::xPosChanged, jsfSpinX, &QSpinBox::setValue);

      QObject::connect(
        jsfSpinY, &QSpinBox::valueChanged, jsfStick, &Joystick::setYPos);
      QObject::connect(
        jsfStick, &Joystick::yPosChanged, jsfSpinY, &QSpinBox::setValue);
    }
    jsfLayout->setColumnStretch(0, 0);
    jsfLayout->setColumnStretch(1, 1);
    rootLayout->addWidget(jsFrame);
  }

  void MainWindow::setupButtonFrame() {
    btnFrame   = new QGroupBox(this);
    btnfLayout = new QGridLayout(btnFrame);

    btnFrame->setTitle("Buttons");
    {
      auto setupButton = [&](
                           QPushButton*& btn, const QString& label, int row,
                           int col, int rowSpan = 1, int colSpan = 1) {
        btn = new QPushButton(this);
        btn->setCheckable(true);
        btn->setText(label);
        btn->setSizePolicy(QSizePolicy {
          QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding});
        btn->setStyleSheet(BUTTON_STYLING);

        btnfLayout->addWidget(btn, row, col, rowSpan, colSpan);
      };

      setupButton(btnfButtonL, "L", 0, 0, 1, 3);
      setupButton(btnfButtonZ, "Z", 0, 3, 1, 3);
      setupButton(btnfButtonR, "R", 0, 6, 1, 3);

      setupButton(btnfButtonDU, "▲", 1, 1);
      setupButton(btnfButtonDD, "▼", 3, 1);
      setupButton(btnfButtonDL, "◀", 2, 0);
      setupButton(btnfButtonDR, "▶", 2, 2);
      labelD = new QLabel(this);
      labelD->setText(QStringLiteral("D"));
      labelD->setAlignment(Qt::AlignCenter);
      btnfLayout->addWidget(labelD, 2, 1);

      setupButton(btnfButtonCU, "▲", 1, 7);
      setupButton(btnfButtonCD, "▼", 3, 7);
      setupButton(btnfButtonCL, "◀", 2, 6);
      setupButton(btnfButtonCR, "▶", 2, 8);
      labelC = new QLabel(this);
      labelC->setText(QStringLiteral("C"));
      labelC->setAlignment(Qt::AlignCenter);
      btnfLayout->addWidget(labelC, 2, 7);

      setupButton(btnfButtonS, "S", 2, 4);
      setupButton(btnfButtonB, "B", 4, 5);
      setupButton(btnfButtonA, "A", 5, 6);

      // Setup a spacer widget
      btnfSpacerR3 = new QWidget(this);
      btnfSpacerR3->setStyleSheet(BUTTON_STYLING);
      btnfLayout->addWidget(btnfSpacerR3, 2, 3);
    }
    // make rows equally sized
    for (size_t i = 0; i < btnfLayout->rowCount(); i++)
      btnfLayout->setRowStretch(i, 1);
    for (size_t i = 0; i < btnfLayout->columnCount(); i++)
      btnfLayout->setColumnStretch(i, 1);
    // add button frame to parent
    rootLayout->addWidget(btnFrame);
  }

  MainWindow::~MainWindow() {}
}  // namespace tnp
