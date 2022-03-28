#pragma once
#include <QGraphicsItem>
#include <optional>

#include "graphics_text_item.h"
#include "graphics_line_item.h"

class GraphicsItem : public QGraphicsItem {
  enum class Type {
    Line
  };

private:
  Type type_ = Type::Line;
  bool selected_ = false;
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  std::optional<qreal> calib_coef_;

  QList<QPointF> points_;
  GraphicsLineItem* line_ = nullptr;
  GraphicsTextItem* item_ = nullptr;

public:
  static GraphicsItem* makeLine(const QPointF& p1, const QPointF& p2, QGraphicsItem* parent);

public:
  GraphicsItem(QGraphicsItem* parent = nullptr);
  GraphicsItem(const QLineF& line, QGraphicsItem* parent = nullptr);
  ~GraphicsItem();

  double length() const;
  bool isSelected() const;
  bool isPartUnderPos(const QPointF& point) const;
  bool isUnderPos(const QPointF& point) const;
  bool isItemUnderMouse() const;
  bool isValid() const;

  void setCalibrationCoef(std::optional<qreal> coef);
  void setScaleFactor(float scale_factor);
  void setSelected(bool selected);
  
  bool checkSelection(const QPointF& pos);
  void updateCaption();
  void updateColors();

  void mousePressEvent(const QPointF& pos);
  void mouseReleaseEvent(const QPointF& pos);
  void mouseMoveEvent(const QPointF& pos);

  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
