#include "graphics_poly_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

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

bool GraphicsPolyItem::isHighlighted() const {
  return highlighted_;
}

bool GraphicsPolyItem::isUnderPos(const QPointF& p) const {
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

  painter->setPen(QPen(Qt::red, 1.0 / scale_factor_));

  auto poly = polygon();
  for (int k = 0; k < poly.count(); ++k) {
    const auto pt = poly[k].toPoint();
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLine(pt - QPoint(5 / scale_factor_, 0), pt + QPoint(5 / scale_factor_, 0)));
    painter->drawLine(QLine(pt - QPoint(0, 5 / scale_factor_), pt + QPoint(0, 5 / scale_factor_)));
  }
}
