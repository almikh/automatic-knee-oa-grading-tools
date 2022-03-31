#pragma once
#include <QGraphicsEllipseItem>

class GraphicsEllipseItem : public QGraphicsEllipseItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;

public:
  GraphicsEllipseItem(const QRectF& rect, QGraphicsItem* parent = nullptr);

  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;

  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
