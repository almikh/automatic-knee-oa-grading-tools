#pragma once
#include <vector>
#include <exception>
#include <memory>

#include <opencv2/opencv.hpp>

namespace tfdetect
{
  struct Detection {
    unsigned int class_id;
    float confidence;

    float x_min;
    float y_min;
    float x_max;
    float y_max;

    Detection(unsigned int class_id, float confidence,
      float x_min, float y_min, float x_max, float y_max) :
      class_id(class_id),
      confidence(confidence),
      x_min(x_min),
      y_min(y_min),
      x_max(x_max),
      y_max(y_max) 
    {

    }
  };

  class Detector
  {
  public:
    virtual ~Detector() = default;
    virtual void detect(const cv::Mat& input_image, std::vector<Detection>& results) const = 0;
  };

  std::unique_ptr<Detector> CreateDetectorFromGraph(const std::string& path_to_graph_proto);

} // namespace tf_detector
