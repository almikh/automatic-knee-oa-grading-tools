#include "smart_curve_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

SmartCurveItem::SmartCurveItem(const QVector<QPoint>& points, QGraphicsItem* parent) :
  QGraphicsItem(parent),
  points_(points)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

void SmartCurveItem::setPoint(int idx, const QPoint& pt) {
  points_[idx] = pt;
  redraw();
}

bool SmartCurveItem::isHighlighted() const {
  return highlighted_;
}

bool SmartCurveItem::isUnderPos(const QPointF& p) const {
  return false;
}

QVector<QPoint> SmartCurveItem::points() const {
  return points_;
}

void SmartCurveItem::setPoints(const QVector<QPoint>& points) {
  points_ = points;
  redraw();
}

void SmartCurveItem::setGradient(const cv::Mat_<double>& gradient) {
  path_finder_ = PathFinder(gradient.size());
  path_finder_.setGradient(gradient);
  path_finder_.scaleGradient(0, 10);
}

void SmartCurveItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
}

void SmartCurveItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void SmartCurveItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void SmartCurveItem::setPen(const QPen& pen) {
  pen_ = pen;
}

QRectF SmartCurveItem::boundingRect() const {
  return QRectF();
}

void SmartCurveItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  for (int k = 0; k < points_.count(); ++k) {
    const auto pt = points_[k];
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLine(pt - QPoint(5 / scale_factor_, 0), pt + QPoint(5 / scale_factor_, 0)));
    painter->drawLine(QLine(pt - QPoint(0, 5 / scale_factor_), pt + QPoint(0, 5 / scale_factor_)));
  }

  painter->setPen(pen_);
  for (const auto& path : path_) {
    painter->drawPoints(path.data(), path.size());
  }
}

void SmartCurveItem::redraw() {
  if (part_under_mouse_ < 0) {
    path_.clear();
    for (int k = 1; k < points_.size(); ++k) {
      if (path_finder_.find(cv::Point(points_[k - 1].x(), points_[k - 1].y()), cv::Point(points_[k].x(), points_[k].y()))) {
        QVector<QPointF> path;
        for (auto pt : path_finder_.lastPath()) {
          path.push_back(QPointF(pt.x, pt.y));
        }

        path_.push_back(path);
      }
    }
  }
  else {
    for (auto k : { part_under_mouse_ , part_under_mouse_ +1 }) {
      if (0 < k && k < points_.size() && path_finder_.find(cv::Point(points_[k - 1].x(), points_[k - 1].y()), cv::Point(points_[k].x(), points_[k].y()))) {
        QVector<QPointF> path;
        for (auto pt : path_finder_.lastPath()) {
          path.push_back(QPointF(pt.x, pt.y));
        }

        path_[k-1] = path;
      }
    }
  }

  update();
}
