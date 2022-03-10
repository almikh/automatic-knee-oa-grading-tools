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
  autoscale_policy_(AutoscalePolicy::MinFactor),
  label_(new QGraphicsTextItem()),
  autoscale_(false),
  scale_factor_(1.0) 
{
  setMouseTracking(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  label_->setFont(QFont("Arial", 17));
  label_->setDefaultTextColor(QColor(206, 6, 52));
}

QHBoxLayout* Viewport::extraLayout() {
  return extra_layout_;
}

double Viewport::scaleFactor() {
  return scale_factor_;
}

void Viewport::setScale(qreal factor) {
  pixmap_item_->setScale(1.0 / scale_factor_);
  scale_factor_ = factor;

  pixmap_item_->setScale(scale_factor_);
}

void Viewport::setAutoscalePolicy(Viewport::AutoscalePolicy policy) {
  autoscale_policy_ = policy;
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

  if (!scene()) {
    auto s = new QGraphicsScene();
    s->setBackgroundBrush(QBrush(qRgb(0, 0, 0)));
    QGraphicsView::setScene(s);
    setStyleSheet("");

    //
    last_pixmap_ = QPixmap::fromImage(image);
    pixmap_item_ = scene()->addPixmap(last_pixmap_);
    pixmap_item_->setPos(-0.5, -0.5);

    scene()->setSceneRect(0, 0, width(), height());
    scene()->addItem(label_);

    updateScale();
    return;
  }

  last_pixmap_ = QPixmap::fromImage(image);
  pixmap_item_->setPixmap(last_pixmap_);
}

void Viewport::fitImageToViewport() {
  if (!last_pixmap_.isNull() && scene()) {
    pixmap_item_->setScale(1.0 / scale_factor_);
    scale_factor_ = double(width()) / scene()->width() * 0.95;
    scale_factor_ = qMin(scale_factor_, double(height()) / scene()->height() * 0.95);
    pixmap_item_->setScale(scale_factor_);

    scene()->setSceneRect(0, 0, width(), height());
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

void Viewport::updateScale() {
  if (last_pixmap_.isNull() || !scene()) return;

  if (autoscale_ && !size().isEmpty()) {
    pixmap_item_->setScale(1.0 / scale_factor_);
    scale_factor_ = double(width()) / last_pixmap_.width() * 0.95;
    if (autoscale_policy_ == AutoscalePolicy::MinFactor) {
      scale_factor_ = qMin(scale_factor_, double(height()) / last_pixmap_.height() * 0.95);
    }
    else {
      scale_factor_ = qMax(scale_factor_, double(height()) / last_pixmap_.height() * 0.95);
    }

    auto w1 = width(), h1 = height();
    auto w2 = last_pixmap_.width() * scale_factor_;
    auto h2 = last_pixmap_.height() * scale_factor_;

    pixmap_item_->setScale(scale_factor_);
    pixmap_item_->setPos((w1 - w2) * 0.5, (h1 - h2) * 0.5);
  }

  scene()->setSceneRect(0, 0, width(), height());
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
  updateScale();
}

void Viewport::wheelEvent(QWheelEvent* event) {
  if (!scene()) return;

  if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
    if (!autoscale_) {
      qreal factor = std::pow(1.001, event->delta());
      scale_factor_ *= factor;

      auto w1 = width(), h1 = height();
      auto w2 = last_pixmap_.width() * scale_factor_;
      auto h2 = last_pixmap_.height() * scale_factor_;

      pixmap_item_->setScale(scale_factor_);
      pixmap_item_->setPos((w1 - w2) * 0.5, (h1 - h2)*0.5);

      scene()->setSceneRect(0, 0, width(), height());

      emit scaleChanged(scale_factor_);
    }
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
        pixmap_item_->setPos(pt);
        repaint();
      }
    }
    else {
      auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
      if ((last_sent_pos_ - coord).manhattanLength() > 2) {
        emit mousePosChanged(coord.toPoint());
        last_sent_pos_ = coord;
      }
    }
  }
  else emit mousePosOutOfImage();
}

void Viewport::setAutoscaleEnabled(bool autoscale) {
  autoscale_ = autoscale;
  updateScale();
}
