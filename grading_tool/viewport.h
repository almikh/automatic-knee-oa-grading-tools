#pragma once
#include <optional>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QVector>

#include <opencv2/opencv.hpp>
#include "graphics_text_item.h"
#include "graphics_line_item.h"

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
    DrawLine
  };
  struct State {
    double scale = 1.0;
    QPointF position;
  };

protected:
  Mode mode_ = Mode::View;
  QPixmap last_pixmap_;
  QGraphicsTextItem* label_ = nullptr;
  QGraphicsPixmapItem* pixmap_item_ = nullptr;
  QList<GraphicsLineItem*> lines_;
  QMap<QGraphicsItem*, GraphicsTextItem*> plates_;
  bool drawing_ = false;

  qreal scale_factor_;
  QPointF last_sent_pos_;
  std::optional<QPointF> anchor_shift_;

public:
  explicit Viewport(QWidget* parent = nullptr);

  Mode mode() const;
  State state() const;
  double scaleFactor() const;

  void setMode(Mode mode);
  void setState(State state);

  void setLabelText(const QString& text);
  void setLabelVisible(bool visible);

  void clearScene();
  void scaleBy(qreal mult);
  void scaleTo(qreal mult);
  void setImage(const cv::Mat& image);
  void setImage(const QImage& image);

  void resizeEvent(QResizeEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

public:
  Q_SLOT void fitImageToViewport();

  Q_SIGNAL void signalOnClick(const QPointF&);
  Q_SIGNAL void mousePosChanged(const QPoint&);
  Q_SIGNAL void mousePosOutOfImage();
  Q_SIGNAL void scaleChanged(qreal);
};
