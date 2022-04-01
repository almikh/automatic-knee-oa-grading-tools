#include "graphics_text_item.h"
#include <QPainter>

GraphicsTextItem::GraphicsTextItem(const QString& text, QGraphicsItem* parent) :
  QGraphicsTextItem(text, parent) 
{
  setDefaultTextColor(Qt::yellow);
}

void GraphicsTextItem::setBackgroundColor(QColor color) {
  background_color_ = color;
}

void GraphicsTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QColor pen(background_color_.red(), background_color_.green(), background_color_.blue());
  QFontMetrics metrics(font());

  auto rect = boundingRect();
  painter->setPen(QPen(pen, 1.0));
  painter->setBrush(QBrush(background_color_));
  painter->drawRect(rect);

  QGraphicsTextItem::paint(painter, o, w);
}
