#include "titlebar.hpp"
#include <qapplication.h>
#include <qfontmetrics.h>
#include <qnamespace.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>
#include <qstyleoption.h>
#include <QEvent>
#include <QSizePolicy>
#include <QStyleOptionTitleBar>
#include <QStylePainter>
#include <QWidget>
#include <QWindow>
#include <optional>
#include <QMouseEvent>

#include <config.hpp>

namespace {
  const QString displayName = QStringLiteral(TNP_DISPLAY_NAME);
}

namespace tnp {
  TitleBar::TitleBar(QMainWindow* parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy {QSizePolicy::MinimumExpanding, QSizePolicy::Minimum});
  }
  TitleBar::~TitleBar() {}

  QSize TitleBar::sizeHint() const {
    QStyleOptionButton btnOpts;
    btnOpts.features = QStyleOptionButton::None;
    btnOpts.text = displayName;
    
    QFontMetrics metrics = fontMetrics();
    QSize size = metrics.size(Qt::TextSingleLine, displayName);
    size = style()->sizeFromContents(QStyle::CT_PushButton, &btnOpts, size);
    
    return size;
  }

  void TitleBar::paintEvent(QPaintEvent*) {
    QStylePainter pnt(this);
    QStyleOption opt;
    opt.rect = rect().adjusted(-2, -2, 2, 0);
    
    pnt.drawPrimitive(QStyle::PE_PanelButtonCommand, opt);
    pnt.drawItemText(rect(), Qt::AlignVCenter | Qt::AlignHCenter, palette(), true, displayName, QPalette::ButtonText);
  }
  void TitleBar::mousePressEvent(QMouseEvent* event) {
    
    switch (event->button()) {
      case Qt::LeftButton:
        topLevelWidget()->windowHandle()->startSystemMove();
        break;
      default: break;
    }
  }
}  // namespace tnp