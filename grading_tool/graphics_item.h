#pragma once
#include <QGraphicsPixmapItem>
#include <QJsonObject>
#include <optional>

#include "defs.h"
#include "graphics_text_item.h"
#include "graphics_line_item.h"
#include "graphics_ellipse_item.h"
#include "graphics_angle_item.h"
#include "graphics_poly_item.h"
#include <opencv2/opencv.hpp>

class GraphicsItem : public QGraphicsItem {
public:
  static float base_touch_radius;

  enum class Type {
    Ellipse,
    Line,
    Angle,
    Poly
  };

private:
  Type type_ = Type::Line;
  bool selected_ = false;
  bool highlighted_ = false;
  float scale_factor_ = 1.0f;
  std::optional<qreal> calib_coef_;
  std::optional<int> avg_, min_, max_, area_, perimeter_;
  bool created_ = false;

  QList<QPointF> points_;
  GraphicsLineItem* line_ = nullptr;
  GraphicsEllipseItem* ellipse_ = nullptr;
  GraphicsAngleItem* angle_ = nullptr;
  GraphicsPolyItem* poly_ = nullptr;
  GraphicsTextItem* item_ = nullptr;

  int anchor_index_ = -1;

public:
  bool h_flipped = false;
  bool v_flipped = false;
  int rotation = 0;

public:
  static GraphicsItem* makeLine(const QPointF& p1, const QPointF& p2, QGraphicsPixmapItem* parent);
  static GraphicsItem* makeEllipse(const QPointF& p1, const QPointF& p2, QGraphicsPixmapItem* parent);
  static GraphicsItem* makeAngle(const QPointF& pt, QGraphicsPixmapItem* parent);
  static GraphicsItem* makePoly(const QPointF& pt, QGraphicsPixmapItem* parent);

  static GraphicsItem* makeFromJson(const QJsonObject& data, const QVector<Transformation>& t, int r, QGraphicsPixmapItem* parent);

public:
  GraphicsItem(QGraphicsItem* parent = nullptr);
  ~GraphicsItem();

  Type getType() const;
  QPolygonF polygon() const;
  QJsonObject toJson() const;

  double length() const;
  bool isSelected() const;
  bool isPartUnderPos(const QPointF& point) const;
  bool isUnderPos(const QPointF& point) const;
  bool isItemUnderMouse() const;
  bool isCreated() const;
  bool isValid() const;

  void setPolygon(const QPolygonF& poly);
  void setCalibrationCoef(std::optional<qreal> coef);
  void setScaleFactor(float scale_factor);
  void setCreated(bool created, std::optional<int> rotation = std::nullopt);
  void setSelected(bool selected);

  bool checkSelection(const QPointF& pos);
  void checkPartUnderPos(const QPointF& pos);
  void updateCaption();
  void updateColors();

  void mousePressEvent(const QPointF& pos);
  void mouseReleaseEvent(const QPointF& pos);
  void mouseMoveEvent(const QPointF& pos, const cv::Mat& image);

  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) override;
};
