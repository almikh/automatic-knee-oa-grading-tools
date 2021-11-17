#include "utils.h"

namespace convert {
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

