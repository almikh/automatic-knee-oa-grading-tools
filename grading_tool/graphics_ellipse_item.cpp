#include "graphics_ellipse_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

GraphicsEllipseItem::GraphicsEllipseItem(const QRectF& rect, QGraphicsItem* parent) :
  QGraphicsEllipseItem(rect, parent)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

void GraphicsEllipseItem::setPoint(int idx, const QPointF& pt) {
  auto r = rect();
  if (idx == 0) r.setBottomLeft(pt);
  else if (idx == 1) r.setTopLeft(pt);
  else if (idx == 2) r.setTopRight(pt);
  else if (idx == 3) r.setBottomRight(pt);

  setRect(r);
}

bool GraphicsEllipseItem::isHighlighted() const {
  return highlighted_;
}

bool GraphicsEllipseItem::isUnderPos(const QPointF& p) const {
  auto center = rect().center();
  auto r1 = (rect().right() - rect().left()) / 2;
  auto r2 = (rect().top() - rect().bottom()) / 2;
  auto dist = std::sqrt(std::pow(center.x() - p.x(), 2) + std::pow(center.y() - p.y(), 2));

  return r1 - 3 < dist && dist < r1 + 3;
}

void GraphicsEllipseItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
}

void GraphicsEllipseItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void GraphicsEllipseItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void GraphicsEllipseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsEllipseItem::paint(painter, o, w);

  auto r = rect();
  auto tick_length = qMax(1.0f, 5 / scale_factor_);
  painter->setPen(QPen(Qt::red, 1.0 / scale_factor_));

  // painter->setBrush(QBrush(Qt::NoBrush));
  // painter->drawRect(r);

  painter->setPen(QPen(part_under_mouse_ == 0 ? Qt::green : Qt::red, 1.0 / scale_factor_));
  painter->drawLine(QLineF(r.bottomLeft().toPoint() - QPointF(tick_length, 0), r.bottomLeft().toPoint() + QPointF(tick_length, 0)));
  painter->drawLine(QLineF(r.bottomLeft().toPoint() - QPointF(0, tick_length), r.bottomLeft().toPoint() + QPointF(0, tick_length)));

  painter->setPen(QPen(part_under_mouse_ == 1 ? Qt::green : Qt::red, 1.0 / scale_factor_));
  painter->drawLine(QLineF(r.topLeft().toPoint() - QPointF(tick_length, 0), r.topLeft().toPoint() + QPointF(tick_length, 0)));
  painter->drawLine(QLineF(r.topLeft().toPoint() - QPointF(0, tick_length), r.topLeft().toPoint() + QPointF(0, tick_length)));

  painter->setPen(QPen(part_under_mouse_ == 2 ? Qt::green : Qt::red, 1.0 / scale_factor_));
  painter->drawLine(QLineF(r.topRight().toPoint() - QPointF(tick_length, 0), r.topRight().toPoint() + QPointF(tick_length, 0)));
  painter->drawLine(QLineF(r.topRight().toPoint() - QPointF(0, tick_length), r.topRight().toPoint() + QPointF(0, tick_length)));

  painter->setPen(QPen(part_under_mouse_ == 3 ? Qt::green : Qt::red, 1.0 / scale_factor_));
  painter->drawLine(QLineF(r.bottomRight().toPoint() - QPointF(tick_length, 0), r.bottomRight().toPoint() + QPointF(tick_length, 0)));
  painter->drawLine(QLineF(r.bottomRight().toPoint() - QPointF(0, tick_length), r.bottomRight().toPoint() + QPointF(0, tick_length)));
}
