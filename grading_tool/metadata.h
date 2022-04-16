#pragma once
#include <memory>
#include <optional>
#include <QString>
#include <QVector>

#include <opencv2/opencv.hpp>
#include "classifier.h"
#include "viewport.h"

struct Metadata {
  using HardPtr = std::shared_ptr<Metadata>;
  struct Joint {
    cv::Rect rect;
    QVector<Classifier::Item> grades;
  };

  enum Rotation {
    Rotate90_CW = 90,
    Rotate90_CCW = -90,
    Rotate180 = 180,
  };

  enum Transformation {
    HFlip = 0,
    VFlip = 1
  };

public:
  cv::Mat image;
  cv::Mat src_image;
  QString filename;
  QVector<Joint> joints;
  bool already_display = false;
  Viewport::State viewport_state;
  std::optional<qreal> calib_coef;
  QJsonArray graphics_items;
  QVector<Transformation> transformations;
  int rotation = 0;
};
