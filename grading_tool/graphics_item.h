#pragma once
#include <QGraphicsItem>

#include "graphics_text_item.h"
#include "graphics_line_item.h"

class GraphicsItem : public QGraphicsItem {
  enum class Type {
    Line
  };

private:
  Type type_ = Type::Line;
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;

  QList<QPointF> points_;
  GraphicsLineItem* line_ = nullptr;
  GraphicsTextItem* item_ = nullptr;

public:
  static GraphicsItem* makeLine(const QPointF& p1, const QPointF& p2, QGraphicsItem* parent);

public:
  GraphicsItem(QGraphicsItem* parent = nullptr);
  GraphicsItem(const QLineF& line, QGraphicsItem* parent = nullptr);
  ~GraphicsItem();

  bool isHighlighted() const;
  bool isPartUnderPos(const QPointF& point) const;
  bool isUnderPos(const QPointF& point) const;
  bool isValid() const;

  void setPen(const QColor& color);
  void setScaleFactor(float scale_factor);
  void setHighlighted(bool selected);
  
  void checkSelection(const QPointF& pos);

  void mousePressEvent(const QPointF& pos);
  void mouseReleaseEvent(const QPointF& pos);
  void mouseMoveEvent(const QPointF& pos);

  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
