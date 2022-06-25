/*
Copyright (c) 2017 Pavel Perina
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// Pavel's code was adapted to fit the patterns of design I use in this project.

#ifndef TNP_ASPECT_LAYOUT_HPP_
#define TNP_ASPECT_LAYOUT_HPP_

#include <qnamespace.h>
#include <qsize.h>
#include <QLayout>
#include <QLayoutItem>
#include <QWidget>
#include <Qt>

namespace tnp {
  // Layout that contains a single item and maintains its aspect ratio.
  class AspectLayout : public QLayout {
    Q_OBJECT
  public:
    AspectLayout(QWidget* parent = nullptr, qreal aspectRatio = 1) :
      QLayout(parent), m_item(nullptr), m_aspectRatio(aspectRatio) {}

    virtual ~AspectLayout() {
      if (m_item != nullptr)
        delete m_item;
    }

    void setAspectRatio(qreal aspectRatio) { m_aspectRatio = aspectRatio; }

    int count() const override { return int(m_item != nullptr); }

    QLayoutItem* itemAt(int i) const override {
      return i == 0 ? m_item : nullptr;
    }

    QLayoutItem* takeAt(int i) override {
      QLayoutItem* res = m_item;
      m_item           = nullptr;
      return res;
    }

    Qt::Orientations expandingDirections() const override {
      return Qt::Horizontal | Qt::Vertical;
    }

    bool hasHeightForWidth() const override { return false; }

    int heightForWidth(int width) const override {
      const auto margins = contentsMargins();
      return (width - (margins.left() + margins.right())) / m_aspectRatio +
        (margins.top() + margins.bottom());
    }

    void setGeometry(const QRect& rect) override {
      QLayout::setGeometry(rect);
      const auto margins = contentsMargins();

      if (m_item) {
        QWidget* wdg = m_item->widget();
        int availW   = rect.width() - (margins.left() + margins.right());
        int availH   = rect.height() - (margins.top() + margins.bottom());
        int w, h;
        h = availH;
        w = h * m_aspectRatio;
        if (w > availW) {
          // fill width
          w = availW;
          h = w / m_aspectRatio;
          int y;
          if (m_item->alignment() & Qt::AlignTop)
            y = margins.top();
          else if (m_item->alignment() & Qt::AlignBottom)
            y = rect.height() - margins.bottom() - h;
          else
            y = margins.top() + (availH - h) / 2;
          wdg->setGeometry(rect.x() + margins.left(), rect.y() + y, w, h);
        }
        else {
          int x;
          if (m_item->alignment() & Qt::AlignLeft)
            x = margins.left();
          else if (m_item->alignment() & Qt::AlignRight)
            x = rect.width() - margins.right() - w;
          else
            x = margins.left() + (availW - w) / 2;
          wdg->setGeometry(rect.x() + x, rect.y() + margins.left(), w, h);
        }
      }
    }

    QSize sizeHint() const override {
      const auto margins = contentsMargins();

      auto res = QSize(
        margins.left() + margins.right(), margins.top() + margins.bottom());
      if (m_item)
        res += m_item->sizeHint();
      return res;
    }

    QSize minimumSize() const override {
      const auto margins = contentsMargins();

      auto res = QSize(
        margins.left() + margins.right(), margins.top() + margins.bottom());
      if (m_item)
        res += m_item->minimumSize();
      return res;
    }

    void addItem(QLayoutItem* item) override {
      if (item != nullptr)
        delete m_item;
      m_item = item;
      item->setAlignment(Qt::Alignment(0));
    }
    
  private:
    QLayoutItem* m_item;
    qreal m_aspectRatio;
  };
}  // namespace tnp

#endif