#include "graphics_line_item.h"
#include <QPainter>

GraphicsLineItem::GraphicsLineItem(const QLineF& line, QGraphicsItem* parent) :
  QGraphicsLineItem(line, parent)
{

}

void GraphicsLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsLineItem::paint(painter, o, w);
  
  painter->setPen(QPen(Qt::black, 1.0));
  painter->setBrush(QBrush(pen().color()));
  painter->drawEllipse(line().p1(), 3, 3);
  painter->drawEllipse(line().p2(), 3, 3);
}
