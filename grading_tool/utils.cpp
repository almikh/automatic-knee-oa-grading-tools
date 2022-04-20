#include "utils.h"

namespace convert 
{
  QImage cv2qt(cv::Mat image) {
    const auto format = QImage::Format::Format_RGB888;
    const auto image_step = static_cast<int>(image.step);
    QImage temp(image.data, image.cols, image.rows, image_step, format);
    temp.bits(); // force run deep copy

    return temp;
  }

  cv::Mat qt2cv(QImage const& imgsrc) {
    QImage   swapped = imgsrc.rgbSwapped();
    cv::Mat temp(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine());
    cv::Mat result;
    return temp.clone();
  }

  QPointF adjustToRect(QPointF point, const QRectF& rect) {
    if (point.x() < 0) point.setX(0);
    if (point.y() < 0) point.setY(0);
    if (point.x() >= rect.width()) point.setX(rect.width() - 1);
    if (point.y() >= rect.height()) point.setY(rect.height() - 1);

    return point;
  }
}

QPointF rotatedPoint(const QPointF& pt, int angle, QSize sz, bool enabled) {
  if (angle == 0) {
    return pt;
  }
  else if (enabled) {
    if (angle == 90) return QPointF(sz.width() - pt.y(), pt.x());
    else if (angle == 180) return QPointF(sz.width() - pt.x(), sz.height() - pt.y());
    else if (angle == 270) return QPointF(pt.y(), sz.height() - pt.x());
  }
  else { // disabled
    if (angle == 90) return QPointF(pt.y(), sz.width() - pt.x());
    else if (angle == 180) return QPointF(sz.width() - pt.x(), sz.height() - pt.y());
    else if (angle == 270) return QPointF(sz.height() - pt.y(), pt.x());
  }

  return pt;
}

qreal dist(const QPointF& p1, const QPointF& p2) {
  return std::sqrt((p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y()));
}

QString point2str(const QPointF& pt) {
  return QString::number(pt.x(), 'f', 2) + ";" + QString::number(pt.y(), 'f', 2);
}

QPointF str2point(const QString& str) {
  auto vals = str.split(';');
  return QPointF(vals[0].toDouble(), vals[1].toDouble());
}
