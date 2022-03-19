#pragma once
#include <QGraphicsTextItem>

class GraphicsTextItem : public QGraphicsTextItem {
  QColor background_color_ = qRgba(214, 169, 56, 32);

public:
  GraphicsTextItem(const QString& text, QGraphicsItem* parent = nullptr);

  void setBackgroundColor(QColor color);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
