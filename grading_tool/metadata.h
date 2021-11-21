#pragma once
#include <memory>
#include <QString>
#include <QVector>

#include <opencv2/opencv.hpp>
#include "classifier.h"

struct Metadata {
  using HardPtr = std::shared_ptr<Metadata>;
  struct Joint {
    cv::Rect rect;
    QVector<Classifier::Item> grades;
  };

public:
  cv::Mat image;
  QString filename;
  QVector<Joint> joints;
};
