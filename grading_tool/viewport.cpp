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

const QColor colors[] = {
   QColor(Qt::red),
   QColor(Qt::blue),
};

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

std::optional<qreal> Viewport::calibCoef() const {
  return calib_coef_;
}

QJsonArray Viewport::graphicsItems() const {
  QJsonArray ans;
  for (auto item : graphics_items_) {
    if (item->isValid()) ans.push_back(item->toJson());
  }

  return ans;
}

Viewport::Mode Viewport::mode() const {
  return mode_;
}

ViewportState Viewport::state() const {
  ViewportState s;
  s.scale = scale_factor_;
  if (pixmap_item_) {
    s.position = pixmap_item_->pos();
  }

  return s;
}

void Viewport::removeGraphicsItem(GraphicsItem* item) {
  auto idx = graphics_items_.indexOf(item);
  if (idx >= 0) {
    graphics_items_.removeAt(idx);
    scene()->removeItem(item);
    // delete item;

    repaint();
  }
}

void Viewport::scaleTo(qreal factor) {
  auto prev_pos = QPoint((width() - last_pixmap_.width() * scale_factor_) * 0.5, (height() - last_pixmap_.height() * scale_factor_) * 0.5);
  auto prev_scale_factor = scale_factor_;

  scale_factor_ = factor;

  auto new_pos = QPoint((width() - last_pixmap_.width() * scale_factor_) * 0.5, (height() - last_pixmap_.height() * scale_factor_) * 0.5);

  pixmap_item_->setScale(scale_factor_);
  pixmap_item_->setPos(pixmap_item_->pos() + (new_pos - prev_pos));

  for (auto item : graphics_items_) {
    item->setScaleFactor(scale_factor_);
  }

  emit scaleChanged(scale_factor_);
}

void Viewport::scaleBy(qreal mult) {
  scaleTo(scale_factor_ * mult);
}

void Viewport::resetCalibrationCoef() {
  calib_coef_ = std::nullopt;

  for (auto item : graphics_items_) {
    item->setCalibrationCoef(std::nullopt);
  }

  repaint();
}

void Viewport::setCalibrationCoef(std::optional<qreal> coef) {
  calib_coef_ = coef;

  for (auto item : graphics_items_) {
    item->setCalibrationCoef(coef);
  }

  repaint();
}

void Viewport::setMode(Viewport::Mode mode) {
  mode_ = mode;
  if (mode == Viewport::Mode::View && !graphics_items_.isEmpty()) {
    auto item = graphics_items_.last();
    if (!item->isValid()) {
      scene()->removeItem(item);
      graphics_items_.pop_back();
      delete item;
    }
    else item->setCreated(true, rotation_);
  }
}

void Viewport::setState(ViewportState state) {
  scale_factor_ = state.scale;
  pixmap_item_->setScale(scale_factor_);
  pixmap_item_->setPos(state.position);
}

void Viewport::setGraphicsItems(const QJsonArray& items, const QVector<Transformation>& t, int r) {
  for (auto json : items) {
    auto item = GraphicsItem::makeFromJson(json.toObject(), t, r, pixmap_item_);
    item->setCalibrationCoef(calib_coef_);
    item->setScaleFactor(scale_factor_);
    item->setCreated(true);

    graphics_items_.push_back(item);
  }

  repaint();
}

void Viewport::addNewSmartCurve(const QVector<QPoint>& points) {
  auto item = GraphicsItem::makeSmartCurve(points.first(), pixmap_item_);
  item->setCalibrationCoef(calib_coef_);
  item->setScaleFactor(scale_factor_);
  item->setGradient(gradient_);
  item->setPoints(points);
  item->setCreated(true);
  item->calcParameters(image_);
  item->updateCaption();

  graphics_items_.push_back(item);

  repaint();
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
  resetCalibrationCoef();

  // remove previous graphics items
  for (auto item : graphics_items_) {
    scene()->removeItem(item);
  }

  graphics_items_.clear();
  // clicks_counter_ = 0;
  // drawing_ = false;

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
}

void Viewport::setJoints(const QVector<Metadata::Joint>& joints, const QVector<Transformation>& transforms, int r) {
  if (auto s = scene()) {
    for (auto item : joints_items_) {
      s->removeItem(item);
    }

    joints_items_.clear();
  }
  
  int counter = 0;
  auto sz = pixmap_item_->pixmap().size();
  for (auto joint : joints) {
    auto item = new QGraphicsRectItem(pixmap_item_);
    auto rect = QRectF(joint.rect.x, joint.rect.y, joint.rect.size().width, joint.rect.size().height);
    auto tl = rotatedPoint(rect.topLeft(), r, sz), br = rotatedPoint(rect.bottomRight(), r, sz);

    for (auto t : transforms) {
      if (t == Transformation::HFlip) {
        tl.setX(sz.width() - tl.x());
        br.setX(sz.width() - br.x());
      }
      else if (t == Transformation::VFlip) {
        tl.setY(sz.height() - tl.y());
        br.setY(sz.height() - br.y());
      }
    }

    item->setRect(QRectF(tl, br));
    item->setPen(QPen(QBrush(colors[counter++ % 2]), 3 / scale_factor_));

    joints_items_.push_back(item);
  }
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

    for (auto item : graphics_items_) {
      item->setScaleFactor(scale_factor_);
    }
  }
}

void Viewport::setImage(const cv::Mat& image, int rotation) {
  rotation_ = rotation;

  if (image.type() != CV_8UC3) {
    cv::cvtColor(image, image_, cv::COLOR_GRAY2RGB);
    setImage(convert::cv2qt(image_));
  }
  else {
    image_ = image;
    setImage(convert::cv2qt(image));
  }
}

void Viewport::setGradient(const cv::Mat& gradient) {
  gradient_ = gradient;
}

void Viewport::clearScene() {
  setStyleSheet("background-color: black;");
  if (auto s = scene()) {
    for (auto item : s->items()) {
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

void Viewport::mouseDoubleClickEvent(QMouseEvent* event) {
  QGraphicsView::mouseDoubleClickEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
  if (event->button() == Qt::LeftButton) {
    if (drawing_) {
      if (mode_ == Mode::DrawPoly) {
        auto item = graphics_items_.last();
        auto poly = item->polygon();
        if (poly.size() > 2 && dist(poly[poly.count() - 2], poly.back()) <= 7) {
          poly.pop_back();
          item->setPolygon(poly);
        }

        if (!item->isValid()) {
          scene()->removeItem(item);
          graphics_items_.pop_back();
          delete item;
        }
        else item->setCreated(true, rotation_);

        clicks_counter_ = 0;
        drawing_ = false;
        repaint();
      }
      else if (mode_ == Mode::SmartCurve) {
        auto item = graphics_items_.last();
        auto points = item->points();
        if (points.size() > 2 && dist(points[points.size() - 2], points.back()) <= 3) {
          points.pop_back();
          item->setPoints(points);
        }

        if (!item->isValid()) {
          scene()->removeItem(item);
          graphics_items_.pop_back();
          delete item;
        }
        else item->setCreated(true, rotation_);

        clicks_counter_ = 0;
        drawing_ = false;
        repaint();
      }
    }
    else {
      GraphicsItem* item = nullptr;
      for (int k = 0; k < graphics_items_.size(); ++k) {
        const auto t = graphics_items_[k]->getType();
        if ((t == GraphicsItem::Type::SmartCurve || t == GraphicsItem::Type::Poly) && graphics_items_[k]->isUnderPos(coord, true)) {
          item = graphics_items_[k];
          break;
        }
      }

      if (item) {
        item->addExtraPoint(coord);
        item->setSelected(false);
      }
    }
  }
}

void Viewport::mousePressEvent(QMouseEvent* event) {
  QGraphicsView::mousePressEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  auto rect = QRectF(pixmap_item_->pos() + QPointF(1, 1), (last_pixmap_.size() - QSize(2, 2)) * scale_factor_);
  if (rect.contains(point)) {
    emit signalOnClick(point - pixmap_item_->pos());
  }

  if (event->button() == Qt::RightButton) {
    if (rect.contains(point)) {
      anchor_shift_ = pixmap_item_->pos() - point;
      setCursor(Qt::ClosedHandCursor);
    }
  }
  else if (event->button() == Qt::LeftButton) {
    GraphicsItem* item = nullptr;
    auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
    if (mode_ == Mode::Calibrate) {
      item = GraphicsItem::makeLine(coord, coord, pixmap_item_);
      item->setScaleFactor(scale_factor_);
      item->setFixedColor(Qt::blue);

      graphics_items_.push_back(item);
      drawing_ = true;
    }
    else {
      GraphicsItem* under_selection = nullptr;
      for (int k = 0; k < graphics_items_.size(); ++k) {
        if (!graphics_items_[k]->isCreated()) {
          item = graphics_items_.takeAt(k);
          break;
        }
        else if (graphics_items_[k]->isPartUnderPos(coord)) {
          item = graphics_items_.takeAt(k);
          break;
        }
        else if (graphics_items_[k]->isUnderPos(coord)) {
          under_selection = graphics_items_[k];
          break;
        }
      }

      // create new if no selected line
      if (!item) {
        if (under_selection) {
          if (under_selection->isSelected()) {
            under_selection->setSelected(false);
          }
          else {
            for (auto item : graphics_items_) {
              item->setSelected(false);
            }

            under_selection->setSelected(true);
          }
        }
        else if (mode_ == Mode::DrawLine) {
          item = GraphicsItem::makeLine(coord, coord, pixmap_item_);
          item->setCalibrationCoef(calib_coef_);
          item->setScaleFactor(scale_factor_);
          graphics_items_.push_back(item);
          drawing_ = true;
        }
        else if (mode_ == Mode::DrawAngle) {
          item = GraphicsItem::makeAngle(coord, pixmap_item_);
          item->setCalibrationCoef(calib_coef_);
          item->setScaleFactor(scale_factor_);
          graphics_items_.push_back(item);
          drawing_ = true;
        }
        else if (mode_ == Mode::DrawCobbAngle) {
          item = GraphicsItem::makeCobbAngle(coord, pixmap_item_);
          item->setCalibrationCoef(calib_coef_);
          item->setScaleFactor(scale_factor_);
          graphics_items_.push_back(item);
          clicks_counter_ = 0;
          drawing_ = true;
        }
        else if (mode_ == Mode::DrawPoly) {
          item = GraphicsItem::makePoly(coord, pixmap_item_);
          item->setCalibrationCoef(calib_coef_);
          item->setScaleFactor(scale_factor_);
          graphics_items_.push_back(item);
          drawing_ = true;
        }
        else if (mode_ == Mode::DrawCircle) {
          item = GraphicsItem::makeEllipse(coord, coord, pixmap_item_);
          item->setCalibrationCoef(calib_coef_);
          item->setScaleFactor(scale_factor_);
          graphics_items_.push_back(item);
          drawing_ = true;
        }
        else if (mode_ == Mode::SmartCurve) {
          item = GraphicsItem::makeSmartCurve(coord, pixmap_item_);
          item->setCalibrationCoef(calib_coef_);
          item->setScaleFactor(scale_factor_);
          item->setGradient(gradient_);
          graphics_items_.push_back(item);
          drawing_ = true;
        }
      }
      else {
        item->mousePressEvent(coord);
        graphics_items_.push_back(item);
        drawing_ = true;
      }
    }

    repaint();
  }
}

void Viewport::mouseReleaseEvent(QMouseEvent* event) {
  QGraphicsView::mouseReleaseEvent(event);
  if (!scene()) return;

  setCursor(Qt::ArrowCursor);

  auto point = mapToScene(event->pos());
  auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
  if (event->button() == Qt::RightButton) {
    if (pixmap_item_) {
      auto next_shift = pixmap_item_->pos() - point;
      if (!anchor_shift_ || (next_shift - anchor_shift_.value()).manhattanLength() <= 2) {
        for (int k = 0; k < graphics_items_.size(); ++k) {
          if (graphics_items_[k]->isItemUnderMouse()) {
            emit menuForItemRequested(graphics_items_[k], event->pos());
            break;
          }
        }
      }
      
      anchor_shift_ = std::nullopt;
    }
  }
  else if (drawing_) {
    if (mode_ == Mode::Calibrate) {
      auto item = graphics_items_.last();
      scene()->removeItem(item);
      graphics_items_.pop_back();      
      if (item->isValid()) {
        emit calibFinished(item->length());
      }

      delete item;

      setMode(Mode::View);
    }
    else if (mode_ == Mode::DrawAngle) {
      auto item = graphics_items_.last();
      auto poly = item->polygon();
      if (poly.count() < 3) {
        if (dist(poly[0], poly[1]) > 7) {
          poly.append(coord);
          item->setPolygon(poly);
        }

        // NOTE:
        // continue drawing
        repaint();
        return;
      }
      else if (!item->isValid()) {
        scene()->removeItem(item);
        graphics_items_.pop_back();
        delete item;
      }
      else item->setCreated(true, rotation_);
    }
    else if (mode_ == Mode::DrawCobbAngle) {
      auto item = graphics_items_.last();
      if (!item->isCreated()) {
        auto poly = item->polygon();
        clicks_counter_ += 1;

        if (clicks_counter_ == 1) item->setPolygon(poly);
        else if (clicks_counter_ > 1) {
          poly.append(coord);
          item->setPolygon(poly);
          repaint();
        }

        if (clicks_counter_ == 4) {
          item->setCreated(true, rotation_);
        }
        else return;
      }
    }
    else if (mode_ == Mode::DrawPoly) {
      auto item = graphics_items_.last();
      if (!item->isCreated()) {
        auto poly = item->polygon();
        if (dist(poly[0], poly[1]) > 7) {
          poly.append(coord);
          item->setPolygon(poly);
        }

        // NOTE:
        // continue drawing
        repaint();
        return;
      }
    }
    else if (mode_ == Mode::SmartCurve) {
      auto item = graphics_items_.last();
      if (!item->isCreated()) {
        auto points = item->points();
        points.push_back(coord.toPoint());
        item->setPoints(points);

        // NOTE:
        // continue drawing
        repaint();
        return;
      }
    }
    else if (mode_ == Mode::DrawLine || mode_ == Mode::DrawCircle) {
      auto item = graphics_items_.last();
      if (!item->isValid()) {
        scene()->removeItem(item);
        graphics_items_.pop_back();
        delete item;
      }
      else item->setCreated(true, rotation_);
    }
    else {
      for (auto item : graphics_items_) {
        item->checkSelection(coord);
      }
    }

    clicks_counter_ = 0;
    drawing_ = false;
    repaint();
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
  QGraphicsView::mouseMoveEvent(event);
  if (!scene()) return;

  auto point = mapToScene(event->pos());
  auto rect = QRectF(pixmap_item_->pos() + QPointF(1, 1), (last_pixmap_.size() - QSize(2, 2)) * scale_factor_);
  if (pixmap_item_ && anchor_shift_) { // move image
    if (rect.contains(point)) {
      auto pt = anchor_shift_.value() + point;
      if ((pt - pixmap_item_->pos()).manhattanLength() > 3) {
        auto shift = pt - pixmap_item_->pos();
        pixmap_item_->setPos(pt);
        repaint();
      }
    }
  }
  else if (pixmap_item_) {
    auto coord = QPointF(point - pixmap_item_->pos()) / scale_factor_;
    if (drawing_) {
      if (!graphics_items_.isEmpty() && rect.contains(point)) {
        auto item = graphics_items_.last();
        item->mouseMoveEvent(coord, image_);
      }
    }
    else {
      bool found = false;
      for (int k = 0; k < graphics_items_.size(); ++k) {
        if (!graphics_items_[k]->isSelected()) {
          if (graphics_items_[k]->checkSelection(coord)) {
            found = true;
            break;
          }
        }
      }

      if (!found) {
        for (int k = 0; k < graphics_items_.size(); ++k) {
          if (graphics_items_[k]->checkPartUnderPos(coord)) {
            break;
          }
        }
      }

      if (rect.contains(point)) {
        if ((last_sent_pos_ - coord).manhattanLength() > 2) {
          emit mousePosChanged(coord.toPoint());
          last_sent_pos_ = coord;
        }
      }
      else emit mousePosOutOfImage();
    }

    repaint();
  }
}
