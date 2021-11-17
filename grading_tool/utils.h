#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>

namespace convert 
{
  QImage cv2qt(cv::Mat image);

  cv::Mat qt2cv(QImage const& imgsrc);

  QPointF adjustToRect(QPointF point, const QRectF& rect);
}
