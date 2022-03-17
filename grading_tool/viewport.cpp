#include "viewport.h"
#include <QApplication>
#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPixmap>
#include <QBrush>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QPainter>

#include "utils.h"

Viewport::Viewport(QWidget* parent) :
  QGraphicsView(parent),
  label_(new QGraphicsTextItem()),
  scale_factor_(1.0) 
{
  setMouseTracking(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  label_->setFont(QFont("Arial", 17));
  label_->setDefaultTextColor(QColor(206, 6, 52));
}

double Viewport::scaleFactor() const {
  return scale_factor_;
}

Viewport::Mode Viewport::mode() const {
  return mode_;
}

Viewport::State Viewport::state() const {
  State s;
  s.scale = scale_factor_;
  if (pixmap_item_) {
    s.position = pixmap_item_->pos();
  }

  return s;
}

void Viewport::scaleTo(qreal factor) {
  auto prev_pos = QPoint((width() - last_pixmap_.width() * scale_factor_) * 0.5, (height() - last_pixmap_.height() * scale_factor_) * 0.5);
  auto prev_scale_factor = scale_factor_;

  scale_factor_ = factor;

  auto new_pos = QPoint((width() - last_pixmap_.width() * scale_factor_) * 0.5, (height() - last_pixmap_.height() * scale_factor_) * 0.5);

  pixmap_item_->setScale(scale_factor_);
  pixmap_item_->setPos(pixmap_item_->pos() + (new_pos - prev_pos));

  for (auto line : lines_) {
    line->setPen(QPen(Qt::red, 2 / scale_factor_));
  }

  emit scaleChanged(scale_factor_);
}

void Viewport::scaleBy(qreal mult) {
  scaleTo(scale_factor_ * mult);
}

void Viewport::setMode(Viewport::Mode mode) {
  mode_ = mode;
}

void Viewport::setState(Viewport::State state) {
  scale_factor_ = state.scale;
  pixmap_item_->setScale(scale_factor_);
  pixmap_item_->setPos(state.position);
}

void Viewport::setLabelText(const QString& text) {
  label_->setPos(QPointF(8, height() - 40));
  label_->setPlainText(text);
  setLabelVisible(true);
}

void Viewport::setLabelVisible(bool visible) {
  if (label_->isVisible() != visible) {
    label_->setVisible(visible);
  }
}

void Viewport::setImage(const QImage& image) {
  // если изменился размер изображения
  if (!last_pixmap_.isNull() && last_pixmap_.size() != image.size()) {
    clearScene();
  }

  last_pixmap_ = QPixmap::fromImage(image);

  if (!scene()) {
    auto s = new QGraphicsScene();
    s->setBackgroundBrush(QBrush(qRgb(0, 0, 0)));
    s->setSceneRect(0, 0, width(), height());
    QGraphicsView::setScene(s);
    setStyleSheet("");

    pixmap_item_ = scene()->addPixmap(last_pixmap_);
    s->addItem(label_);
  }
  else {
    pixmap_item_->setPixmap(last_pixmap_);
  }

  fitImageToViewport();
}

void Viewport::fitImageToViewport() {
  if (!last_pixmap_.isNull() && scene()) {
    scale_factor_ = qMin(
      double(width()) / last_pixmap_.width() * 0.95,
      double(height()) / last_pixmap_.height() * 0.95);

    auto w2 = last_pixmap_.width() * scale_factor_;
    auto h2 = last_pixmap_.height() * scale_factor_;

    pixmap_item_->setScale(scale_factor_);
    pixmap_item_->setPos((width() - w2) * 0.5, (height() - h2) * 0.5);
  }
}

void Viewport::setImage(const cv::Mat& image) {
  if (image.type() != CV_8UC3) {
    cv::Mat rgb;
    cv::cvtColor(image, rgb, cv::COLOR_GRAY2RGB);
    setImage(convert::cv2qt(rgb));
  }
  else setImage(convert::cv2qt(image));
}

void Viewport::clearScene() {
  setStyleSheet("background-color: black;");
  if (auto s = scene()) {
    for (auto& item : s->items()) {
      if (!dynamic_cast<QGraphicsPixmapItem*>(item)) {
        s->removeItem(item);
      }
    }

    delete s;

    QGraphicsView::setScene(nullptr);
  }
}

void Viewport::resizeEvent(QResizeEvent* event) {
  QGraphicsView::resizeEvent(event);

  if (scene()) {
    scene()->setSceneRect(0, 0, width(), height());
  }
}

void Viewport::wheelEvent(QWheelEvent* event) {
  if (!scene()) return;

  if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
    qreal factor = std::pow(1.001, event->delta());
    scaleBy(factor);
  }
  else QGraphicsView::wheelEvent(event);
}

void Viewport::mousePressEvent(QMouseEvent* event) {
  QGraphicsView::mousePressEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  auto rect = QRectF(pixmap_item_->pos() + QPointF(1, 1), (last_pixmap_.size() - QSize(2, 2)) * scale_factor_);
  if (rect.contains(point)) {
    emit signalOnClick(point - pixmap_item_->pos());

    if (event->button() == Qt::RightButton) {
      anchor_shift_ = pixmap_item_->pos() - point;
      setCursor(Qt::ClosedHandCursor);
    }
    else if (mode_ == Mode::DrawLine) {
      auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
      auto line = new QGraphicsLineItem(QLineF(coord, coord), pixmap_item_);
      line->setPen(QPen(Qt::red, 2 / scale_factor_));
      line->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);
      line->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, true);
      line->setAcceptTouchEvents(true);
      lines_.push_back(line);
      drawing_ = true;
    }
  }
}

void Viewport::mouseReleaseEvent(QMouseEvent* event) {
  QGraphicsView::mouseReleaseEvent(event);
  if (!scene()) return;

  setCursor(Qt::ArrowCursor);

  auto point = mapToScene(event->pos());
  if (event->button() == Qt::RightButton) {
    if (pixmap_item_ && anchor_shift_) {
      pixmap_item_->setPos(anchor_shift_.value() + point);
      anchor_shift_ = std::nullopt;
    }
  }
  else if (drawing_) {
    if (mode_ == Mode::DrawLine) {
      auto item = lines_.last();
      if (item->line().length() < 3) {
        scene()->removeItem(item);
        lines_.pop_back();
        delete item;
      }
    }

    drawing_ = false;
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
  QGraphicsView::mouseMoveEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  auto rect = QRectF(pixmap_item_->pos() + QPointF(1, 1), (last_pixmap_.size() - QSize(2, 2)) * scale_factor_);
  if (rect.contains(point)) {
    // point = convert::adjustToRect(point, scene()->sceneRect());
    if (pixmap_item_ && anchor_shift_) {
      auto pt = anchor_shift_.value() + point;
      if ((pt - pixmap_item_->pos()).manhattanLength() > 3) {
        auto shift = pt - pixmap_item_->pos();
        pixmap_item_->setPos(pt);
        repaint();
      }
    }
    else {
      auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
      if (drawing_) {
        if (mode_ == Mode::DrawLine && !lines_.isEmpty()) {
          auto line = lines_.last()->line();
          line.setP2(QPointF(coord));
          lines_.last()->setLine(line);
          repaint();
        }
      }
      else {
        if ((last_sent_pos_ - coord).manhattanLength() > 2) {
          emit mousePosChanged(coord.toPoint());
          last_sent_pos_ = coord;
        }
      }
    }
  }
  else emit mousePosOutOfImage();
}
