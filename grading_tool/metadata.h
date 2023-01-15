#pragma once
#include <memory>
#include <optional>
#include <QJsonArray>
#include <QString>
#include <QVector>

#include <defs.h>
#include <opencv2/opencv.hpp>
#include "classifier.h"
#include "types.h"

struct Metadata {
  using HardPtr = std::shared_ptr<Metadata>;
  struct Joint {
    cv::Rect rect;
    QVector<Classifier::Item> grades;
  };

public:
  cv::Mat image;
  cv::Mat src_image;
  cv::Mat gradient;
  QString filename;
  QVector<Joint> joints;
  xr::contours_t contours;
  bool already_display = false;
  ViewportState viewport_state;
  std::optional<qreal> calib_coef;
  QJsonArray graphics_items;
  QVector<Transformation> transformations;
  int rotation = 0;
};
