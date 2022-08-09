#pragma once
#include <QGraphicsLineItem>

class GraphicsPolyItem : public QGraphicsPolygonItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  int part_under_mouse_ = -1;

public:
  GraphicsPolyItem(const QPolygonF& poly, QGraphicsItem* parent = nullptr);

  void addExtraPoint(const QPointF& point);
  void setPoint(int idx, const QPointF& pt);

  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point, float* out) const;

  void setPartUnderMouse(int idx);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
