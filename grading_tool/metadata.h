#pragma once
#include <memory>
#include <QString>

#include <opencv2/opencv.hpp>

struct Metadata {
  using HardPtr = std::shared_ptr<Metadata>;
public:
  cv::Mat image;
  QString filename;
};
