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

namespace convert {
  QImage cv2qt(cv::Mat image) {
    const auto format = QImage::Format::Format_RGB888;
    const auto image_step = static_cast<int>(image.step);
    QImage temp(image.data, image.cols, image.rows, image_step, format);
    temp.bits(); // force run deep copy

    return temp;
  }

  cv::Mat qt2cv(QImage const& imgsrc) {
    QImage   swapped = imgsrc.rgbSwapped();
    cv::Mat temp(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine());
    cv::Mat result;
    return temp.clone();
  }

  QPointF adjustToRect(QPointF point, const QRectF& rect) {
    if (point.x() < 0) point.setX(0);
    if (point.y() < 0) point.setY(0);
    if (point.x() >= rect.width()) point.setX(rect.width() - 1);
    if (point.y() >= rect.height()) point.setY(rect.height() - 1);

    return point;
  }
}

Viewport::Viewport(QWidget* parent) :
  QGraphicsView(parent),
  roi_item_(new QGraphicsRectItem()),
  marker_item_(new QGraphicsEllipseItem()),
  marker_center_item_(new QGraphicsEllipseItem()),
  scene_rect_item_(new QGraphicsRectItem()),
  autoscale_policy_(AutoscalePolicy::MinFactor),
  mouse_cursor_text(new QGraphicsTextItem()),
  editable_marker(false),
  editable_roi(false),
  autoscale_(true),
  scale_factor_(1.0) {
  setMouseTracking(true);
  roi_item_->setPen(QPen(Qt::red));
  roi_item_->setBrush(QColor(255, 255, 255, 8));

  QPen pen(Qt::blue);
  pen.setStyle(Qt::PenStyle::SolidLine);
  scene_rect_item_->setPen(pen);

  marker_item_->setPen(QPen(Qt::PenStyle::NoPen));
  marker_item_->setBrush(QColor(255, 0, 0, 128));

  marker_center_item_->setPen(QPen(Qt::PenStyle::NoPen));
  marker_center_item_->setBrush(QColor(255, 0, 0, 255));

  mouse_cursor_text->setFont(QFont("Arial", 16));
  mouse_cursor_text->setDefaultTextColor(QColor(255, 255, 255));

}

QHBoxLayout* Viewport::extraLayout() {
  return extra_layout_;
}

double Viewport::scaleFactor() {
  return scale_factor_;
}

void Viewport::setScale(qreal factor) {
  scale(1.0 / scale_factor_, 1.0 / scale_factor_);
  scale_factor_ = factor;

  scale(scale_factor_, scale_factor_);
}

void Viewport::setBorderColor(QColor color) {
  auto pen = scene_rect_item_->pen();
  pen.setColor(color);
  scene_rect_item_->setPen(pen);
}

void Viewport::setBorderStyle(Qt::PenStyle style) {
  auto pen = scene_rect_item_->pen();
  pen.setStyle(style);
  scene_rect_item_->setPen(pen);
}

void Viewport::setAutoscalePolicy(Viewport::AutoscalePolicy policy) {
  autoscale_policy_ = policy;
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

    mouse_cursor_text->setScale(1.0 / scale_factor_);

    scene()->setSceneRect(0, 0, last_pixmap_.width() - 1, last_pixmap_.height() - 1);
    scene()->addItem(mouse_cursor_text);
    scene()->addItem(scene_rect_item_);
    scene()->addItem(marker_center_item_);
    scene()->addItem(marker_item_);
    scene()->addItem(roi_item_);
    /*for (int k = 0; k < selection_items_.size(); k++) {
      scene()->addItem(selection_items_[k]);
    }*/
    scene_rect_item_->setRect(-0.5, -0.5, last_pixmap_.width(), last_pixmap_.height());

    updateScale();
    return;
  }

  last_pixmap_ = QPixmap::fromImage(image);
  pixmap_item_->setPixmap(last_pixmap_);
}

void Viewport::fitImageToViewport() {
  if (!last_pixmap_.isNull() && scene()) {
    scale(1.0 / scale_factor_, 1.0 / scale_factor_);
    scale_factor_ = double(width()) / scene()->width() * 0.95;
    scale_factor_ = qMin(scale_factor_, double(height()) / scene()->height() * 0.95);
    scale(scale_factor_, scale_factor_);

    scene()->setSceneRect(0, 0, last_pixmap_.width() - 1, last_pixmap_.height() - 1);
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
    scale(1.0 / scale_factor_, 1.0 / scale_factor_);
    scale_factor_ = double(width()) / scene()->width() * 0.95;
    if (autoscale_policy_ == AutoscalePolicy::MinFactor) {
      scale_factor_ = qMin(scale_factor_, double(height()) / scene()->height() * 0.95);
    }
    else {
      scale_factor_ = qMax(scale_factor_, double(height()) / scene()->height() * 0.95);
    }

    scale(scale_factor_, scale_factor_);
  }

  scene()->setSceneRect(0, 0, last_pixmap_.width() - 1, last_pixmap_.height() - 1);
}

void Viewport::clearScene() {
  setStyleSheet("background-color: black;");
  if (auto s = scene()) {
    s->removeItem(roi_item_);
    s->removeItem(marker_item_);
    s->removeItem(marker_center_item_);
    s->removeItem(scene_rect_item_);
    s->removeItem(mouse_cursor_text);

    for (auto& item : s->items()) {
      if (!dynamic_cast<QGraphicsPixmapItem*>(item)) {
        s->removeItem(item);
      }
    }

    delete s;

    QGraphicsView::setScene(nullptr);
  }
}

void Viewport::setROI(const QRect& rect) {
  roi = rect;
  origin_point_ = QPoint();
  roi_item_->setRect(rect);
}

void Viewport::resetROI() {
  roi = QRect();
  origin_point_ = QPoint();
  roi_item_->setRect(QRect());
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
      scale(factor, factor);
      scale_factor_ *= factor;

      mouse_cursor_text->setScale(1.0 / scale_factor_);
      scene()->setSceneRect(0, 0, last_pixmap_.width() - 1, last_pixmap_.height() - 1);

      emit scaleChanged(scale_factor_);
    }
  }
  else QGraphicsView::wheelEvent(event);
}

void Viewport::mousePressEvent(QMouseEvent* event) {
  QGraphicsView::mousePressEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  if (!scene()->sceneRect().contains(point)) return;

  emit signalOnClick(point);

  if (event->button() == Qt::LeftButton) {
    if (editable_roi && scene()->sceneRect().contains(point)) {
      origin_point_ = QPoint(point.x(), point.y());
    }
    else if (editable_marker) {
      changeMarkerPos(point);
      emit markerPosChanged(point);
    }
  }
}

void Viewport::mouseReleaseEvent(QMouseEvent* event) {
  QGraphicsView::mouseReleaseEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  if (event->button() == Qt::LeftButton) {
    point = convert::adjustToRect(point, scene()->sceneRect());
    if (editable_roi && origin_point_ != QPoint()) {
      int left = qMin<int>(origin_point_.x(), point.x());
      int top = qMin<int>(origin_point_.y(), point.y());
      int width = qAbs(origin_point_.x() - point.x());
      int height = qAbs(origin_point_.y() - point.y());
      roi_item_->setRect(QRect(left, top, width, height));
      roi = QRect(left, top, width, height);
      origin_point_ = QPoint();

      if (!roi.isEmpty()) {
        emit roiSelected(roi);
      }
    }
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
  QGraphicsView::mouseMoveEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  if (event->buttons() & Qt::LeftButton) {
    point = convert::adjustToRect(point, scene()->sceneRect());
    if (editable_roi && origin_point_ != QPoint()) {
      int left = qMin<int>(origin_point_.x(), point.x());
      int top = qMin<int>(origin_point_.y(), point.y());
      int width = qAbs(origin_point_.x() - point.x());
      int height = qAbs(origin_point_.y() - point.y());
      roi_item_->setRect(QRect(left, top, width, height));
      roi = QRect(left, top, width, height);

      if (!roi.isEmpty()) {
        emit roiChanged(roi);
      }
    }
    else if (editable_marker) {
      changeMarkerPos(point);
      emit markerPosChanged(point);
    }
  }

  emit mousePosChanged(mapToScene(event->pos()));
}

void Viewport::changeROI(const QRect& new_roi) {
  if (editable_roi) {
    roi = new_roi;
    roi_item_->setRect(new_roi);
    origin_point_ = QPoint();
  }
}

void Viewport::changeMarkerPos(const QPointF& pos) {
  marker_pos_ = pos;
  marker_item_->setRect(pos.x() - 1, pos.y() - 1, 2, 2);
  marker_center_item_->setRect(pos.x() - 0.1, pos.y() - 0.1, 0.2, 0.2);

  repaint();
}

void Viewport::setAutoscaleEnabled(bool autoscale) {
  autoscale_ = autoscale;
  updateScale();
}
