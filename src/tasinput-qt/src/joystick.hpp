#ifndef TNP_JOYSTICK_HPP_INCLUDED
#define TNP_JOYSTICK_HPP_INCLUDED

#include <QEvent>
#include <QWidget>

namespace tnp {
  class Joystick : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int xPos MEMBER p_xpos NOTIFY xPosChanged READ xPos WRITE setXPos)
    Q_PROPERTY(int yPos MEMBER p_ypos NOTIFY yPosChanged READ yPos WRITE setYPos)
  public:
    Joystick(QWidget* parent = nullptr);
    ~Joystick();
    
    int xPos() const;
    int yPos() const;
    
    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;
    QSize sizeHint() const override;
    
  Q_SIGNALS:
    void xPosChanged(int value);
    void yPosChanged(int value);
    
  public Q_SLOTS:
    void setXPos(int value);
    void setYPos(int value);
  
  protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    
  private:
    int p_xPos;
    int p_yPos;
    
    void updateJoystickPos(QPointF point);
  };
}

#endif