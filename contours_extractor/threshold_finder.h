#pragma once
#include "defs.h"
#include "session.h"
#include "contours_finder.h"
#include "image.h"

namespace xr
{
  class ThresholdFinder {
  public:
    using HardPtr = std::shared_ptr<ThresholdFinder>;

    struct Item {
      double valuation;
      uint8_t threshold;
      contours_t contours;
      std::shared_ptr<Image> image;
    };

    class Verifier {
    public:
      using HardPtr = std::shared_ptr<Verifier>;

    protected:
      Data::HardPtr data_;
      Image* working_copy_;
      std::shared_ptr<Image> binary_ver_;
      ContoursFinder::HardPtr contours_finder_;

      void removalDiscontinuities();

    public:
      Verifier(Data::HardPtr data, std::shared_ptr<Image> binary);

      void setContoursFinder(ContoursFinder::HardPtr finder);

      Item verify(Image* item, uint8_t threshold);
    };

  protected:
    Data::HardPtr data_;
    int target_index_ = -1;
    std::vector<Item> buffer_;
    std::shared_ptr<Image> binary_ver_;

  public:
    ThresholdFinder(Data::HardPtr data);

    virtual uint8_t find(uint8_t approximation, double step, int count);
    virtual ThresholdFinder::Item& goodImage();
  };

}
