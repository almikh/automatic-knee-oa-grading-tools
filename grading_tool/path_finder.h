#pragma once
#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

class PathFinder {
  int label_ = 0;
  double factor_ = 0.0;
  double medium_ = 0.0;
  std::vector<cv::Point> last_path_;
  cv::Mat_<double> grad_;
  cv::Mat_<int> label_map_;
  cv::Mat_<double> price_, aux_price_;

public:
  using HardPtr = std::shared_ptr<PathFinder>;
  using PointT = std::pair<int, int>;

  enum Mode {
    Distance = 1,
    Gradient = 2,
    GradientDiff = 4
  };

  PathFinder();
  PathFinder(const cv::Size& size);

  bool find(const cv::Point& first, const cv::Point& last, int flag = Distance | Gradient, bool include_init_points = false);
  std::vector<cv::Point> lastPath() const;

  void setGradient(const cv::Mat_<double>& gradient);
  void scaleGradient(double down, double up);
  void setFactor(double factor);
};
