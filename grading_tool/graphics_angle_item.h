#pragma once
#include <QGraphicsLineItem>

class GraphicsAngleItem : public QGraphicsLineItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  QGraphicsLineItem second_line_;
  QPolygonF polygon_;

public:
  GraphicsAngleItem(const QPolygonF& poly, QGraphicsItem* parent = nullptr);

  void setPoint(int idx, const QPointF& pt);

  QPolygonF polygon() const;
  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;
  double angle() const;

  void setPolygon(const QPolygonF& poly);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);
  void setPen(const QPen& pen);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
