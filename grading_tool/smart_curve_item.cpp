#include "smart_curve_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#include "utils.h"

SmartCurveItem::SmartCurveItem(const QVector<QPoint>& points, QGraphicsItem* parent) :
  QGraphicsPathItem(parent),
  points_(points)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
  setBrush(QBrush(Qt::transparent));
}

void SmartCurveItem::setPoint(int idx, const QPoint& pt) {
  points_[idx] = pt;
  redraw();
}

bool SmartCurveItem::isHighlighted() const {
  return highlighted_;
}

bool SmartCurveItem::isUnderPos(const QPointF& p) const {
  for (auto path : path_) {
    for (int k = 0; k < path.size(); k += 3) {
      if (dist(path[k], p) < 7) {
        return true;
      }
    }
  }

  return false;
}

qreal SmartCurveItem::length() const {
  qreal acc = 0.0f;
  QVector<QPointF> prev_segment;
  for (auto path : path_) {
    for (int k = 1; k < path.size(); ++k) {
      acc += dist(path[k - 1], path[k]);
    }

    if (!prev_segment.isEmpty()) {
      acc += dist(prev_segment.last(), path.first());
    }

    prev_segment = path;
  }

  return acc;
}

QVector<QPoint> SmartCurveItem::points() const {
  return points_;
}

void SmartCurveItem::setPoints(const QVector<QPoint>& points) {
  points_ = points;
  redraw();
} 

void SmartCurveItem::addExtraPoint(const QPointF& point) {
  auto min_idx = 0;
  for (int k = 1; k < points_.count(); ++k) {
    if (dist(points_[k], point) < dist(points_[min_idx], point)) {
      min_idx = k;
    }
  }

  auto next_idx = 0;
  if (min_idx == 0) next_idx = min_idx + 1;
  else if (min_idx == points_.size() - 1) next_idx = min_idx - 1;
  else if (dist(points_[min_idx + 1], point) < dist(points_[min_idx - 1], point)) next_idx = min_idx + 1;
  else next_idx = min_idx - 1;

  if (next_idx < min_idx)
    qSwap(min_idx, next_idx);

  points_.insert(min_idx + 1, point.toPoint());
  redraw(true);
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

void SmartCurveItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsPathItem::paint(painter, o, w);

  for (int k = 0; k < points_.count(); ++k) {
    const auto pt = points_[k];
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLine(pt - QPoint(5 / scale_factor_, 0), pt + QPoint(5 / scale_factor_, 0)));
    painter->drawLine(QLine(pt - QPoint(0, 5 / scale_factor_), pt + QPoint(0, 5 / scale_factor_)));
  }
}

void SmartCurveItem::redraw(bool full_redraw) {
  if (part_under_mouse_ < 0 || full_redraw) {
    path_.clear();
    for (int k = 1; k < points_.size(); ++k) {
      if (path_finder_.find(cv::Point(points_[k - 1].x(), points_[k - 1].y()), cv::Point(points_[k].x(), points_[k].y()))) {
        QVector<QPointF> path;
        for (auto pt : path_finder_.lastPath()) {
          path.push_back(QPointF(pt.x, pt.y));
        }

        std::reverse(path.begin(), path.end());
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

  if (!path_.isEmpty()) {
    QPainterPath painter_path;
    for (auto path : path_) {
      painter_path.addPolygon(path);
    }

    setPath(painter_path);
  }
}
