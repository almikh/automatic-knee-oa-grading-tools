#pragma once
#include <optional>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QJsonArray>
#include <QVector>

#include <opencv2/opencv.hpp>
#include "graphics_item.h"
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
    DrawCircle,
    DrawLine,
    DrawAngle,
    DrawPoly
  };
  struct State {
    double scale = 1.0;
    QPointF position;
  };

protected:
  Mode mode_ = Mode::View;
  cv::Mat image_;
  QPixmap last_pixmap_;
  QGraphicsTextItem* label_ = nullptr;
  QGraphicsPixmapItem* pixmap_item_ = nullptr;
  QList<GraphicsItem*> graphics_items_;
  std::optional<qreal> calib_coef_;
  bool drawing_ = false;

  qreal scale_factor_;
  QPointF last_sent_pos_;
  std::optional<QPointF> anchor_shift_;

public:
  explicit Viewport(QWidget* parent = nullptr);

  Mode mode() const;
  State state() const;
  double scaleFactor() const;
  std::optional<qreal> calibCoef() const;
  QJsonArray graphicsItems() const;

  void setMode(Mode mode);
  void setState(State state);

  // add graphics items to viewport
  // @param items:  list of encoded graphics items
  // @param t:      transformations set of current image
  // @param r:      rotation of current image
  void setGraphicsItems(const QJsonArray& items, const QVector<Transformation>& t = {}, int r = 0);

  void setLabelText(const QString& text);
  void setLabelVisible(bool visible);

  void clearScene();
  void scaleBy(qreal mult);
  void scaleTo(qreal mult);
  void resetCalibrationCoef();
  void setCalibrationCoef(std::optional<qreal> coef);
  void removeGraphicsItem(GraphicsItem* item);
  void setImage(const cv::Mat& image);
  void setImage(const QImage& image);

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
  Q_SIGNAL void scaleChanged(qreal);
};
