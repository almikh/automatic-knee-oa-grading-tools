#pragma once
#include <QGraphicsLineItem>

class GraphicsCobbAngleItem : public QGraphicsLineItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  int part_under_mouse_ = -1;
  QGraphicsLineItem second_line_;
  QGraphicsLineItem middle_line_;
  QPolygonF polygon_;

private:
  void initLines(const QPolygonF& poly);

public:
  GraphicsCobbAngleItem(const QPolygonF& poly, QGraphicsItem* parent = nullptr);

  void setPoint(int idx, const QPointF& pt, bool created = false);

  QPolygonF polygon() const;
  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;
  double angle() const;

  void setPolygon(const QPolygonF& poly);
  void setPartUnderMouse(int idx);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);
  void setPen(const QPen& pen);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
