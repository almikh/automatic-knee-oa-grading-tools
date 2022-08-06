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

void scale(cv::Mat& src, double down, double up) {
  double min_value, max_value;
  cv::minMaxLoc(src, &min_value, &max_value);

  double temp = double(up - down) / (max_value - min_value);
  for (int i = 0; i < src.rows; ++i) {
    for (int j = 0; j < src.cols; ++j) {
      auto val = src.at<double>(i, j);
      src.at<double>(i, j) = (down + (val - min_value) * temp);
    }
  }
}

cv::Mat gvf(cv::Mat src, double mu, int iters) {
  cv::Mat_<double> u = cv::Mat_<double>::zeros(src.size());
  cv::Mat_<double> v = cv::Mat_<double>::zeros(src.size());

  cv::Mat_<double> f;
  src.convertTo(f, CV_64FC1); 
  scale(f, 0, 1);

  /* Compute derivative */
  for (int j = 1; j < src.rows - 1; ++j) {
    for (int i = 1; i < src.cols - 1; ++i) {
      u(j, i) = 0.5 * (f(j, i + 1) - f(j, i - 1));
      v(j, i) = 0.5 * (f(j + 1, i) - f(j - 1, i));
    }
  }

  for (int j = 0; j < src.rows; ++j) {
    u(j, 0) = 0.5 * (f(j, 1) - f(j, 0));
    u(j, src.cols - 1) = 0.5 * (f(j, src.cols - 1) - f(j, src.cols - 2));
  }

  for (int i = 0; i < src.cols; ++i) {
    v(0, i) = 0.5 * (f(1, i) - f(0, i));
    v(src.rows - 1, i) = 0.5 * (f(src.rows - 1, i) - f(src.rows - 2, i));
  }

  /* Compute parameters and initializing arrays */
  cv::Mat_<double> b(src.size()), c1(src.size()), c2(src.size());
  for (int j = 0; j < src.rows; ++j) {
    for (int i = 0; i < src.cols; ++i) {
      b(j, i) = sqr(u(j, i)) + sqr(v(j, i));
      c1(j, i) = b(j, i) * u(j, i);
      c2(j, i) = b(j, i) * v(j, i);
    }
  }

  /* Solve GVF = (u,v) */
  cv::Mat_<double> Lu(src.size()), Lv(src.size());
  for (int it = 0; it < iters; ++it) {
    /* corners */
    int n = src.cols - 1;
    int m = src.rows - 1;
    Lu(0, 0) = (2 * u(0, 1) + 2 * u(1, 0)) - 4 * u(0, 0);
    Lv(0, 0) = (2 * v(0, 1) + 2 * v(1, 0)) - 4 * v(0, 0);
    Lu(m, n) = (2 * u(m, n - 1) + u(m - 1, n)) - 4 * u(m, n);
    Lv(m, n) = (2 * v(m, n - 1) + v(m - 1, n)) - 4 * v(m, n);
    Lu(0, n) = (2 * u(0, n - 1) + 2 * u(1, n)) - 4 * u(0, n);
    Lv(0, n) = (2 * v(0, n - 1) + 2 * v(1, n)) - 4 * v(0, n);
    Lu(m, 0) = (2 * u(m, 1) + 2 * u(m - 1, 0)) - 4 * u(m, 0);
    Lv(m, 0) = (2 * v(m, 1) + 2 * v(m - 1, 0)) - 4 * v(m, 0);

    /* interior Lu, Lv*/
    double* uCur, * uPrev, * uNext;
    double* vCur, * vPrev, * vNext;
    double* curLu = (double*)Lu.data;
    double* curLv = (double*)Lv.data;
    for (int j = 1; j < m; ++j) {
      uCur = u[j] + 1;
      uPrev = u[j - 1] + 1;
      uNext = u[j + 1] + 1;
      vCur = v[j] + 1;
      vPrev = v[j - 1] + 1;
      vNext = v[j + 1] + 1;
      curLu = Lu[j] + 1;
      curLv = Lv[j] + 1;
      for (int i = 1; i < n; ++i) {
        *curLu++ = (*(uCur - 1) + *uPrev++ + *(uCur + 1) + *uNext++) - 4 * (*uCur);
        *curLv++ = (*(vCur - 1) + *vPrev++ + *(vCur + 1) + *vNext++) - 4 * (*vCur);
        ++uCur;
        ++vCur;
      }
    }

    /* left and right columns */
    for (int j = 1; j < m; ++j) {
      Lu(j, 0) = (u(j - 1, 0) + 2 * u(j, 1) + u(j + 1, 0)) - 4 * u(j, 0);
      Lv(j, 0) = (v(j - 1, 0) + 2 * v(j, 1) + v(j + 1, 0)) - 4 * v(j, 0);
      Lu(j, n) = (u(j - 1, n) + 2 * u(j, n - 1) + u(j + 1, n)) - 4 * u(j, n);
      Lv(j, n) = (v(j - 1, n) + 2 * v(j, n - 1) + v(j + 1, n)) - 4 * v(j, n);
    }

    /* top and bottom rows */
    for (int i = 1; i < n; ++i) {
      Lu(0, i) = (u(0, i - 1) + 2 * u(1, i) + u(0, i + 1)) - 4 * u(0, i);
      Lv(0, i) = (v(0, i - 1) + 2 * v(1, i) + v(0, i + 1)) - 4 * v(0, i);
      Lu(m, i) = (u(m, i - 1) + 2 * u(m - 1, i) + u(m, i + 1)) - 4 * u(m, i);
      Lv(m, i) = (v(m, i - 1) + 2 * v(m - 1, i) + v(m, i + 1)) - 4 * v(m, i);
    }

    /* Update GVF  */
    double* curU = (double*)u.data;
    double* curV = (double*)v.data;
    double* curb = (double*)b.data;
    double* curC1 = (double*)c1.data;
    double* curC2 = (double*)c2.data;
    curLu = (double*)Lu.data;
    curLv = (double*)Lv.data;
    for (int i = 0, n = src.cols * src.rows; i < n; ++i) {
      *curU = (1.0 - *curb) * (*curU) + mu * (*curLu++) + *curC1++;
      *curV = (1.0 - *curb) * (*curV) + mu * (*curLv++) + *curC2++;
      ++curU;
      ++curV;
      ++curb;
    }
  }

  cv::Mat dst = cv::Mat::zeros(src.size(), CV_64FC1);
  for (int j = 0; j < src.rows; ++j) {
    for (int i = 0; i < src.cols; ++i) {
      dst.at<double>(j, i) = sqrt(sqr(u(j, i)) + sqr(v(j, i)));
    }
  }

  scale(dst, 0, 1);

  return dst;
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

qreal distToLine(const QPointF& p, const QPointF& pa, const QPointF& pb) {
  const auto t = ((p.x() - pa.x()) * (pb.x() - pa.x()) + (p.y() - pa.y()) * (pb.y() - pa.y())) /
    ((pb.x() - pa.x()) * (pb.x() - pa.x()) + (pb.y() - pa.y()) * (pb.y() - pa.y()));

  double length = DBL_MAX;
  if (0 <= t && t <= 1) {
    length = std::sqrt(sqr(pa.x() - p.x() + (pb.x() - pa.x()) * t) + sqr(pa.y() - p.y() + (pb.y() - pa.y()) * t));
  }

  return length;
}

QString point2str(const QPointF& pt) {
  return QString::number(pt.x(), 'f', 2) + ";" + QString::number(pt.y(), 'f', 2);
}

QPointF str2point(const QString& str) {
  auto vals = str.split(';');
  return QPointF(vals[0].toDouble(), vals[1].toDouble());
}

qreal perimeter(const QPolygonF& poly) {
  qreal acc = 0.0f;
  acc += dist(poly.last(), poly.first());
  for (int k = 1; k < poly.count(); ++k) {
    acc += dist(poly[k - 1], poly[k]);
  }

  return acc;
}

qreal square(const QPolygonF& poly) {
  int count = 0;
  auto rect = poly.boundingRect();
  int left = qMax<int>(0, qMin(rect.x(), rect.x() + rect.width()));
  int bottom = qMax<int>(0, qMin(rect.y(), rect.y() + rect.height()));
  int right = left + qAbs<int>(rect.width()), top = bottom + qAbs(rect.height());
  for (int i = left; i <= right; ++i) {
    for (int j = bottom; j <= top; ++j) {
      if (poly.containsPoint(QPointF(i, j), Qt::FillRule::OddEvenFill)) {
        count += 1;
      }
    }
  }

  return count;
}

cv::Rect rect(const xr::contour_t& contour) {
  int left = INT_MAX, right = 0, bottom = INT_MAX, top = 0;
  for (int k = 0; k < contour.size(); ++k) {
    if (contour[k].x < left) left = contour[k].x;
    if (contour[k].x > right) right = contour[k].x;
    if (contour[k].y < bottom) bottom = contour[k].y;
    if (contour[k].y > top) top = contour[k].y;
  }

  return cv::Rect(left, bottom, right - left, top - bottom);
}

qreal max_width(const xr::contour_t& contour) {
  qreal ans = 0.0;
  for (int k = 0; k < contour.size(); ++k) {
    int current_y = contour[k].y;
    for (int k2 = 0; k2 < contour.size(); ++k2) {
      if (k != k2 && contour[k2].y == current_y) {
        ans = qMax<qreal>(ans, qAbs(contour[k2].x - contour[k].x));
      }
    }
  }

  return ans;
}

qreal jaccard(const cv::Rect& lhs, const cv::Rect& rhs) {
  return  (lhs & rhs).area() * 1.0 / (lhs | rhs).area();
}
