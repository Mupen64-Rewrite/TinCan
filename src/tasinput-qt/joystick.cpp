#include "joystick.hpp"
#include <qevent.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <QEvent>
#include <QFrame>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <iostream>

#define with(...) if (__VA_ARGS__; true)

namespace tnp {
  Joystick::Joystick(QWidget* parent) : QWidget(parent), p_xPos(0), p_yPos(0) {
    using repaint_slot_t = void (Joystick::*)();
    // cast is needed to disambiguate overloads
    connect(this, &Joystick::xPosChanged, repaint_slot_t(&Joystick::repaint));
    connect(this, &Joystick::yPosChanged, repaint_slot_t(&Joystick::repaint));

    // set size policy
    QSizePolicy policy {
      QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred};
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
  }

  Joystick::~Joystick() {}

  int Joystick::xPos() const { return p_xPos; }
  void Joystick::setXPos(int value) {
    if (p_xPos == value)
      return;
    p_xPos = value;
    emit Joystick::xPosChanged(value);
  }

  int Joystick::yPos() const { return p_yPos; }
  void Joystick::setYPos(int value) {
    if (p_yPos == value)
      return;
    p_yPos = std::clamp(value, -128, 127);
    emit Joystick::yPosChanged(value);
  }

  bool Joystick::hasHeightForWidth() const { return true; }
  int Joystick::heightForWidth(int w) const { return w; }

  QSize Joystick::sizeHint() const { return QSize {128, 128}; }

  void Joystick::paintEvent(QPaintEvent*) {
    // painting context
    const auto& palette = this->palette();
    QStylePainter pnt(this);

    // more variables
    const auto& rect        = this->rect();
    auto frameOptions       = QStyleOptionFrame();
    frameOptions.frameShape = QFrame::Box;
    frameOptions.lineWidth  = 1;
    frameOptions.rect       = rect;

    const qreal centerX = rect.width() / 2.0;
    const qreal centerY = rect.height() / 2.0;

    // stickX: -128 to +127, left to right
    // stickY: -128 to +127, bottom to top
    // stickY needs to be reversed first before converting offset
    const qreal stickX = (p_xPos + 128) * (rect.width() / 256.0);
    const qreal stickY = (256 - (p_yPos + 128)) * (rect.height() / 256.0);

    // draw background
    pnt.fillRect(rect, palette.base());
    pnt.fillPath(
      [&rect]() -> QPainterPath {
        QPainterPath p;
        p.addEllipse(rect);
        return p;
      }(),
      palette.alternateBase());

    // draw circle and crosshair
    pnt.setPen(QPen {palette.text().color(), 1});
    pnt.drawPath([&rect, centerX, centerY]() -> QPainterPath {
      QPainterPath p;
      p.addEllipse(rect);
      p.moveTo(centerX, 0);
      p.lineTo(centerX, rect.height());
      p.moveTo(0, centerY);
      p.lineTo(rect.width(), centerY);
      return p;
    }());

    // draw line and joystick position
    pnt.setPen(QPen {palette.highlight().color().darker(150), 3});
    pnt.drawLine(QPointF {centerX, centerY}, QPointF {stickX, stickY});
    pnt.fillPath(
      [&rect, stickX, stickY]() -> QPainterPath {
        QPainterPath p;
        p.addEllipse(QRectF {stickX - 5, stickY - 5, 10, 10});
        return p;
      }(),
      palette.highlight());

    // draw wrapping frame
    pnt.drawControl(QStyle::CE_ShapedFrame, frameOptions);
  }

  void Joystick::updateJoystickPos(QPointF point) {
    const QRectF bounds  = QRectF(rect());
    const QPointF center = bounds.center();

    if (!bounds.contains(point)) {
      // vector going outwards from center to point
      const QPointF rel = point - center;
      // clang-format off
      // quadrant of point, use to perform intersection
      // 0, 1, 2, 3 = N, E, S, W
      const uint16_t quad = 
          (std::abs(rel.x()) <= -rel.y()) ? 0
        : (std::abs(rel.y()) <= rel.x()) ? 1
        : (std::abs(rel.x()) <= rel.y()) ? 2
        : 3;
      // clang-format on
      switch (quad) {
        case 0: {
          // fast axis-aligned intersection
          // determine portion of line inside box, then
          // lerp on the x-axis using that portion.
          point.setX(
            //center.x() +
            std::lerp(center.x(), point.x(), center.y() / -rel.y()));
          point.setY(0);
        } break;
        case 1: {
          // same idea as north edge. behold my shitty math.
          point.setX(bounds.width());
          point.setY(
            std::lerp(center.y(), point.y(), center.x() / rel.x()));
        } break;
        case 2: {
          point.setX(
            std::lerp(center.x(), point.x(), center.y() / rel.y()));
          point.setY(bounds.height());
        } break;
        case 3: {
          point.setX(0);
          point.setY(
            std::lerp(center.y(), point.y(), center.x() / -rel.x()));
        } break;
      }
    }
    // convert bounded point to offset
    int nextXPos = std::lerp(-128.0, 127.0, point.x() / bounds.width());
    int nextYPos = std::lerp(127.0, -128.0, point.y() / bounds.height());
    if (nextXPos > -8 && nextXPos < 8) {
      nextXPos = 0;
    }
    if (nextYPos > -8 && nextYPos < 8) {
      nextYPos = 0;
    }
    setXPos(nextXPos);
    setYPos(nextYPos);
  }

  void Joystick::mousePressEvent(QMouseEvent* event) {
    updateJoystickPos(event->position());
    setCursor(QCursor(Qt::PointingHandCursor));
  }
  void Joystick::mouseMoveEvent(QMouseEvent* event) {
    updateJoystickPos(event->position());
  }
  void Joystick::mouseReleaseEvent(QMouseEvent* event) {
    updateJoystickPos(event->position());
    setCursor(QCursor(Qt::ArrowCursor));
  }
}  // namespace tnp