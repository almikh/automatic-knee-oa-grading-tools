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
  painter->setPen(QPen(Qt::red, 1.0 / scale_factor_));

  // painter->setBrush(QBrush(Qt::NoBrush));
  // painter->drawRect(r);

  painter->drawLine(QLine(r.bottomLeft().toPoint() - QPoint(5 / scale_factor_, 0), r.bottomLeft().toPoint() + QPoint(5 / scale_factor_, 0)));
  painter->drawLine(QLine(r.bottomLeft().toPoint() - QPoint(0, 5 / scale_factor_), r.bottomLeft().toPoint() + QPoint(0, 5 / scale_factor_)));

  painter->drawLine(QLine(r.bottomRight().toPoint() - QPoint(5 / scale_factor_, 0), r.bottomRight().toPoint() + QPoint(5 / scale_factor_, 0)));
  painter->drawLine(QLine(r.bottomRight().toPoint() - QPoint(0, 5 / scale_factor_), r.bottomRight().toPoint() + QPoint(0, 5 / scale_factor_)));

  painter->drawLine(QLine(r.topLeft().toPoint() - QPoint(5 / scale_factor_, 0), r.topLeft().toPoint() + QPoint(5 / scale_factor_, 0)));
  painter->drawLine(QLine(r.topLeft().toPoint() - QPoint(0, 5 / scale_factor_), r.topLeft().toPoint() + QPoint(0, 5 / scale_factor_)));

  painter->drawLine(QLine(r.topRight().toPoint() - QPoint(5 / scale_factor_, 0), r.topRight().toPoint() + QPoint(5 / scale_factor_, 0)));
  painter->drawLine(QLine(r.topRight().toPoint() - QPoint(0, 5 / scale_factor_), r.topRight().toPoint() + QPoint(0, 5 / scale_factor_)));
}
