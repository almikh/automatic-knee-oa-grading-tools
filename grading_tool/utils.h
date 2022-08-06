#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>
#include <defs.h>

template<class T>
T sqr(T value) {
  return value * value;
}

void scale(cv::Mat& src, double down, double up);

cv::Mat gvf(cv::Mat src, double mu, int iters);

QPointF rotatedPoint(const QPointF& pt, int angle, QSize pixmap_size, bool enable = true);

qreal dist(const QPointF& p1, const QPointF& p2);
qreal distToLine(const QPointF& p, const QPointF& pa, const QPointF& pb);
qreal perimeter(const QPolygonF& p);
qreal square(const QPolygonF& p);
qreal max_width(const xr::contour_t& contour);
cv::Rect rect(const xr::contour_t& contour);
qreal jaccard(const cv::Rect& lhs, const cv::Rect& rhs);

QString point2str(const QPointF& pt);
QPointF str2point(const QString& str);

namespace convert
{
  QImage cv2qt(cv::Mat image);

  cv::Mat qt2cv(QImage const& imgsrc);

  QPointF adjustToRect(QPointF point, const QRectF& rect);
}
