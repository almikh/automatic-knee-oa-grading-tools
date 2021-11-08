#pragma once
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
  QGraphicsPixmapItem* pixmap_item_ = nullptr;
  QGraphicsRectItem* roi_item_;
  QGraphicsRectItem* scene_rect_item_;
  QGraphicsEllipseItem* marker_item_;
  QGraphicsEllipseItem* marker_center_item_;
  AutoscalePolicy autoscale_policy_;
  QPushButton* save_image_ = nullptr;

  QPoint origin_point_;
  QPointF marker_pos_;
  bool autoscale_;
  qreal scale_factor_;

  void updateScale();

public:
  QGraphicsTextItem* mouse_cursor_text; // текст под курсором мыши
  bool editable_marker; // в окне есть пердвигаемый мышкой маркер
  bool editable_roi; // в области можно мышкой выделять области интереса
  QRect roi;

public:
  explicit Viewport(QWidget* parent = nullptr);

  double scaleFactor();
  QHBoxLayout* extraLayout();

  void setBorderColor(QColor color);
  void setBorderStyle(Qt::PenStyle style);
  void setAutoscalePolicy(AutoscalePolicy policy);

  void clearScene();
  void setImage(const cv::Mat& image);
  void setImage(const QImage& image);
  void fitImageToViewport();

  void resetROI();
  void setROI(const QRect& rect);

  void resizeEvent(QResizeEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

public:
  Q_SLOT void setAutoscaleEnabled(bool autoscale);
  Q_SLOT void changeMarkerPos(const QPointF& pos);
  Q_SLOT void changeROI(const QRect& new_roi);

  Q_SLOT void setScale(qreal scale);

  Q_SIGNAL void signalOnClick(const QPointF&);
  Q_SIGNAL void markerPosChanged(const QPointF& pos);
  Q_SIGNAL void roiSelected(const QRect& new_roi);
  Q_SIGNAL void roiChanged(const QRect& new_roi);
  Q_SIGNAL void mousePosChanged(const QPointF&);
  Q_SIGNAL void scaleChanged(qreal);
};
