#include "graphics_line_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

#include "utils.h"

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
  return distToLine(p, line().p1(), line().p2()) < 5;
}

void GraphicsLineItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
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
  
  painter->setPen(QPen(part_under_mouse_ == 0 ? Qt::green : Qt::red, 1.0 / scale_factor_));
  painter->drawLine(QLine(line().p1().toPoint() - QPoint(5 / scale_factor_, 0), line().p1().toPoint() + QPoint(5 / scale_factor_, 0)));
  painter->drawLine(QLine(line().p1().toPoint() - QPoint(0, 5 / scale_factor_), line().p1().toPoint() + QPoint(0, 5 / scale_factor_)));

  painter->setPen(QPen(part_under_mouse_ == 1 ? Qt::green : Qt::red, 1.0 / scale_factor_));
  painter->drawLine(QLine(line().p2().toPoint() - QPoint(5 / scale_factor_, 0), line().p2().toPoint() + QPoint(5 / scale_factor_, 0)));
  painter->drawLine(QLine(line().p2().toPoint() - QPoint(0, 5 / scale_factor_), line().p2().toPoint() + QPoint(0, 5 / scale_factor_)));
}
