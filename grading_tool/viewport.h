#pragma once
#include <optional>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QVector>

#include <opencv2/opencv.hpp>

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
  enum class AutoscalePolicy {
    MinFactor,
    MaxFactor
  };
  QHBoxLayout* extra_layout_ = nullptr;

protected:
  QPixmap last_pixmap_;
  QGraphicsTextItem* label_ = nullptr;
  QGraphicsPixmapItem* pixmap_item_ = nullptr;
  AutoscalePolicy autoscale_policy_;

  bool autoscale_;
  qreal scale_factor_;
  QPointF last_sent_pos_;
  std::optional<QPointF> anchor_shift_;

  void updateScale();

public:
  explicit Viewport(QWidget* parent = nullptr);

  double scaleFactor();
  QHBoxLayout* extraLayout();

  void setAutoscalePolicy(AutoscalePolicy policy);

  void setLabelText(const QString& text);
  void setLabelVisible(bool visible);

  void clearScene();
  void setImage(const cv::Mat& image);
  void setImage(const QImage& image);
  void fitImageToViewport();

  void resizeEvent(QResizeEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

public:
  Q_SLOT void setAutoscaleEnabled(bool autoscale);
  Q_SLOT void setScale(qreal scale);

  Q_SIGNAL void signalOnClick(const QPointF&);
  Q_SIGNAL void mousePosChanged(const QPoint&);
  Q_SIGNAL void mousePosOutOfImage();
  Q_SIGNAL void scaleChanged(qreal);
};
