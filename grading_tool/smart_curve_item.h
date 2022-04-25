#pragma once
#include <QGraphicsLineItem>

class SmartCurveItem : public QGraphicsItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  int part_under_mouse_ = -1;

public:
  SmartCurveItem(QGraphicsItem* parent = nullptr);

  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;

  void setPartUnderMouse(int idx);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);
  void setPen(const QPen& pen);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
