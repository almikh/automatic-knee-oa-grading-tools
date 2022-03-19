#pragma once
#include <QGraphicsLineItem>

class GraphicsLineItem : public QGraphicsLineItem {
public:
  GraphicsLineItem(const QLineF& line, QGraphicsItem* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
