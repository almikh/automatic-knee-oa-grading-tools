#include "graphics_line_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

double sqr(double val) {
  return val * val;
}

GraphicsLineItem::GraphicsLineItem(const QLineF& line, QGraphicsItem* parent):
  QGraphicsLineItem(line, parent)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

bool GraphicsLineItem::isHighlighted() const {
  return highlighted_;
}

bool GraphicsLineItem::isUnderPos(const QPointF& p) const {
  const auto pa = line().p1(), pb = line().p2();
  const auto t = ((p.x() - pa.x()) * (pb.x() - pa.x()) + (p.y() - pa.y()) * (pb.y() - pa.y())) /
   ((pb.x() - pa.x()) * (pb.x() - pa.x()) + (pb.y() - pa.y()) * (pb.y() - pa.y()));

  double length = DBL_MAX;
  if (0 <= t && t <= 1) {
    length = std::sqrt(sqr(pa.x() - p.x() + (pb.x() - pa.x()) * t) + sqr(pa.y() - p.y() + (pb.y() - pa.y()) * t));
  }

  return length < 5;
}

void GraphicsLineItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void GraphicsLineItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void GraphicsLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsLineItem::paint(painter, o, w);
  
  painter->setPen(QPen(Qt::black, 1.0));
  painter->setBrush(QBrush(pen().color()));
  painter->drawEllipse(line().p1(), 4 / scale_factor_, 4 / scale_factor_);
  painter->drawEllipse(line().p2(), 4 / scale_factor_, 4 / scale_factor_);
}
