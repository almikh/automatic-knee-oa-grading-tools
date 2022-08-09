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

bool SmartCurveItem::isUnderPos(const QPointF& p, float* out) const {
  for (auto path : path_) {
    for (int k = 0; k < path.size(); k += 3) {
      auto dd = dist(path[k], p);
      if (dd < 7) {
        if (out) *out = dd;
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
    if (path.isEmpty())
      continue;

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
  path_finder_.scaleGradient(0, 10.0);
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

  auto tick_length = qMax(1.0f, 5 / scale_factor_);
  for (int k = 0; k < points_.count(); ++k) {
    const auto pt = points_[k];
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLineF(pt - QPointF(tick_length, 0), pt + QPointF(tick_length, 0)));
    painter->drawLine(QLineF(pt - QPointF(0, tick_length), pt + QPointF(0, tick_length)));
  }
}

void SmartCurveItem::redraw(bool full_redraw) {
  if (part_under_mouse_ < 0 || full_redraw) {
    path_.clear();
    for (int k = 1; k <= points_.size(); ++k) {
      int prev_kk = (k - 1  + points_.size()) % points_.size();
      int kk = k % points_.size();

      if (path_finder_.find(cv::Point(points_[prev_kk].x(), points_[prev_kk].y()), cv::Point(points_[kk].x(), points_[kk].y()))) {
        QVector<QPointF> path;
        for (auto pt : path_finder_.lastPath()) {
          path.push_back(QPointF(pt.x, pt.y));
        }

        std::reverse(path.begin(), path.end());
        path_.push_back(path);
      }
      else {
        path_.push_back({});
      }
    }
  }
  else {
    for (auto k : { part_under_mouse_ , part_under_mouse_ +1 }) {
      int prev_kk = (k - 1 + points_.size()) % points_.size();
      int kk = k % points_.size();

      if (0 <= kk && kk < points_.size() && path_finder_.find(cv::Point(points_[prev_kk].x(), points_[prev_kk].y()), cv::Point(points_[kk].x(), points_[kk].y()))) {
        QVector<QPointF> path;
        for (auto pt : path_finder_.lastPath()) {
          path.push_back(QPointF(pt.x, pt.y));
        }

        path_[prev_kk] = path;
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
