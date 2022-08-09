#include "graphics_poly_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#include "utils.h"

GraphicsPolyItem::GraphicsPolyItem(const QPolygonF& poly, QGraphicsItem* parent) :
  QGraphicsPolygonItem(poly, parent)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

void GraphicsPolyItem::setPoint(int idx, const QPointF& pt) {
  auto poly = polygon();
  poly[idx] = pt;

  setPolygon(poly);
}

void GraphicsPolyItem::addExtraPoint(const QPointF& point) {
  auto min_idx = 0;
  auto poly = polygon();
  for (int k = 0; k < poly.count(); ++k) {
    if (dist(poly[k], point) < dist(poly[min_idx], point)) {
      min_idx = k;
    }
  }

  auto next_idx = 0;
  if (min_idx == 0) next_idx = min_idx + 1;
  else if (min_idx == poly.count() - 1) next_idx = min_idx - 1;
  else if (dist(poly[min_idx + 1], point) < dist(poly[min_idx - 1], point)) next_idx = min_idx + 1;
  else next_idx = min_idx - 1;

  if (next_idx < min_idx)
    qSwap(min_idx, next_idx);

  poly.insert(min_idx + 1, point.toPoint());
  setPolygon(poly);
}

bool GraphicsPolyItem::isHighlighted() const {
  return highlighted_;
}

bool GraphicsPolyItem::isUnderPos(const QPointF& p, float* out) const {
  auto poly = polygon();
  for (int k = 1; k < poly.count(); ++k) {
    auto dd = distToLine(p, poly[k], poly[k - 1]);
    if (dd < 5) {
      if (out) *out = dd;
      return true;
    }
  }

  return false;
}

void GraphicsPolyItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
}

void GraphicsPolyItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void GraphicsPolyItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void GraphicsPolyItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsPolygonItem::paint(painter, o, w);

  auto poly = polygon();
  auto tick_length = qMax(1.0f, 5 / scale_factor_);
  for (int k = 0; k < poly.count(); ++k) {
    const auto pt = poly[k].toPoint();
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLineF(pt - QPointF(tick_length, 0), pt + QPointF(tick_length, 0)));
    painter->drawLine(QLineF(pt - QPointF(0, tick_length), pt + QPointF(0, tick_length)));
  }
}
