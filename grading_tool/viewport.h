#pragma once
#include <optional>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QJsonArray>
#include <QVector>

#include <opencv2/opencv.hpp>
#include "graphics_item.h"
#include "metadata.h"
#include "defs.h"

class QPushButton;
class QHBoxLayout;
class QGraphicsPixmapItem;

namespace convert {
  QImage cv2qt(cv::Mat image);
  cv::Mat qt2cv(QImage const& imgsrc);
  QPointF adjustToRect(QPointF point, const QRectF& rect);
}

class Viewport : public QGraphicsView {
  Q_OBJECT

public:
  enum class Mode {
    View,
    Calibrate,
    DrawCircle,
    DrawLine,
    DrawAngle,
    DrawCobbAngle,
    SmartCurve,
    DrawPoly
  };

protected:
  Mode mode_ = Mode::View;
  cv::Mat image_;
  cv::Mat gradient_;
  int rotation_ = 0;
  QPixmap last_pixmap_;
  QGraphicsTextItem* label_ = nullptr;
  QGraphicsPixmapItem* pixmap_item_ = nullptr;
  QList<QGraphicsRectItem*> joints_items_;
  QList<GraphicsItem*> graphics_items_;
  std::optional<qreal> calib_coef_;
  int clicks_counter_ = 0;
  bool drawing_ = false;

  qreal scale_factor_;
  QPointF last_sent_pos_;
  std::optional<QPointF> anchor_shift_;

public:
  explicit Viewport(QWidget* parent = nullptr);

  Mode mode() const;
  ViewportState state() const;
  double scaleFactor() const;
  std::optional<qreal> calibCoef() const;
  QJsonArray graphicsItems() const;

  void setMode(Mode mode);
  void setState(ViewportState state);

  // add graphics items to viewport
  // @param items:  list of encoded graphics items
  // @param t:      transformations set of current image
  // @param r:      rotation of current image
  void setGraphicsItems(const QJsonArray& items, const QVector<Transformation>& t = {}, int r = 0);
  
  // add new smart curve (=contour) from image
  // @param points: points of added contour
  // @param image:  source image
  void addNewSmartCurve(const QVector<QPoint>& points);

  void setLabelText(const QString& text);
  void setLabelVisible(bool visible);

  void clearScene();
  void scaleBy(qreal mult);
  void scaleTo(qreal mult);
  void resetCalibrationCoef();
  void setCalibrationCoef(std::optional<qreal> coef);
  void removeGraphicsItem(GraphicsItem* item);
  void setImage(const cv::Mat& image, int rotation = 0);
  void setGradient(const cv::Mat& gradient);
  void setImage(const QImage& image);
  void setJoints(const QVector<Metadata::Joint>& joints, const QVector<Transformation>& t, int r);

  void resizeEvent(QResizeEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  
public:
  Q_SLOT void fitImageToViewport();

  Q_SIGNAL void signalOnClick(const QPointF&);
  Q_SIGNAL void mousePosChanged(const QPoint&);
  Q_SIGNAL void mousePosOutOfImage();
  Q_SIGNAL void menuForItemRequested(GraphicsItem* item, const QPoint& pos);
  Q_SIGNAL void calibFinished(qreal);
  Q_SIGNAL void scaleChanged(qreal);
};
