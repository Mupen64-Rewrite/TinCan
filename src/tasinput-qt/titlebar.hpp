#ifndef TNP_TITLEBAR_HPP_
#define TNP_TITLEBAR_HPP_

#include <qsize.h>
#include <QMainWindow>
#include <QEvent>
#include <QWidget>
#include <QSize>

namespace tnp {
  class TitleBar : public QWidget {
    Q_OBJECT
  public:
    TitleBar(QMainWindow* parent = nullptr);
    ~TitleBar();
    
    QSize sizeHint() const override;
  protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    
  private:
    QSize autoSizeHint;
  };
}
#endif