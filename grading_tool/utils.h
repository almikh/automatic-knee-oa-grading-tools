#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>

template<class T>
T sqr(T value) {
  return value * value;
}

void scale(cv::Mat& src, double down, double up);

cv::Mat gvf(cv::Mat src, double mu, int iters);

QPointF rotatedPoint(const QPointF& pt, int angle, QSize pixmap_size, bool enable = true);

qreal dist(const QPointF& p1, const QPointF& p2);
QString point2str(const QPointF& pt);
QPointF str2point(const QString& str);

namespace convert
{
  QImage cv2qt(cv::Mat image);

  cv::Mat qt2cv(QImage const& imgsrc);

  QPointF adjustToRect(QPointF point, const QRectF& rect);
}
