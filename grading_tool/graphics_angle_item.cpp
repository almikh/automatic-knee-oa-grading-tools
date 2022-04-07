#include "graphics_angle_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

GraphicsAngleItem::GraphicsAngleItem(const QPolygonF& poly, QGraphicsItem* parent) :
  QGraphicsLineItem(QLineF(poly[0], poly[1]), parent),
  second_line_(this),
  polygon_(poly)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

void GraphicsAngleItem::setPoint(int idx, const QPointF& pt) {
  auto poly = polygon_;
  poly[idx] = pt;

  setPolygon(poly);
}

bool GraphicsAngleItem::isHighlighted() const {
  return highlighted_;
}

double GraphicsAngleItem::angle() const {
  auto ans = second_line_.line().angleTo(QLineF(line().p2(), line().p1()));
  return ans < 180 ? ans : 360 - ans;
}

QPolygonF GraphicsAngleItem::polygon() const {
  return polygon_;
}

void GraphicsAngleItem::setPolygon(const QPolygonF& poly) {
  polygon_ = poly;
  setLine(QLineF(poly[0], poly[1]));
  if (poly.count() > 2) {
    second_line_.setLine(QLineF(poly[1], poly[2]));
  }
}

bool GraphicsAngleItem::isUnderPos(const QPointF& p) const {
  return false;
}

void GraphicsAngleItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
}

void GraphicsAngleItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void GraphicsAngleItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void GraphicsAngleItem::setPen(const QPen& pen) {
  QGraphicsLineItem::setPen(pen);
  second_line_.setPen(pen);
}

void GraphicsAngleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsLineItem::paint(painter, o, w);

  painter->setPen(QPen(Qt::red, 1.0 / scale_factor_));
  
  auto poly = polygon();
  for (int k = 0; k < poly.count(); ++k) {
    const auto pt = poly[k].toPoint();
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLine(pt - QPoint(5 / scale_factor_, 0), pt + QPoint(5 / scale_factor_, 0)));
    painter->drawLine(QLine(pt - QPoint(0, 5 / scale_factor_), pt + QPoint(0, 5 / scale_factor_)));
  }
}
