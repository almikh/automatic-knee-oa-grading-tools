#pragma once
#include <QGraphicsEllipseItem>

class GraphicsEllipseItem : public QGraphicsEllipseItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  int part_under_mouse_ = -1;

public:
  GraphicsEllipseItem(const QRectF& rect, QGraphicsItem* parent = nullptr);

  void setPoint(int idx, const QPointF& pt);

  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;

  void setPartUnderMouse(int idx);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
