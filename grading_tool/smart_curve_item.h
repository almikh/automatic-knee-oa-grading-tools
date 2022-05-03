#pragma once
#include <QGraphicsPolygonItem>
#include <QPen>

#include "path_finder.h"

class SmartCurveItem : public QGraphicsPathItem {
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  int part_under_mouse_ = -1;
  PathFinder path_finder_;
  QVector<QPoint> points_;
  QVector<QVector<QPointF>> path_;

public:
  SmartCurveItem(const QVector<QPoint>& points, QGraphicsItem* parent = nullptr);
  
  void setPoint(int idx, const QPoint& pt);

  bool isHighlighted() const;
  bool isUnderPos(const QPointF& point) const;
  qreal length() const;

  QVector<QPoint> points() const;
  void setPoints(const QVector<QPoint>& points);
  void addExtraPoint(const QPointF& point);

  void setPartUnderMouse(int idx);
  void setGradient(const cv::Mat_<double>& gradient);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
  void redraw(bool full_redraw = false);
};
