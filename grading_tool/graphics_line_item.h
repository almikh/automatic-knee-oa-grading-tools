#pragma once
#include <QGraphicsLineItem>

class GraphicsLineItem : public QGraphicsLineItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;

public:
  GraphicsLineItem(const QLineF& line, QGraphicsItem* parent = nullptr);

  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;

  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
