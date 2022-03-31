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
  painter->setPen(QPen(Qt::black, 1.0));

  painter->setBrush(QBrush(Qt::NoBrush));
  painter->drawRect(r);

  painter->setBrush(QBrush(pen().color()));
  painter->drawEllipse(r.bottomLeft(), 4 / scale_factor_, 4 / scale_factor_);
  painter->drawEllipse(r.bottomRight(), 4 / scale_factor_, 4 / scale_factor_);
  painter->drawEllipse(r.topLeft(), 4 / scale_factor_, 4 / scale_factor_);
  painter->drawEllipse(r.topRight(), 4 / scale_factor_, 4 / scale_factor_);
}
