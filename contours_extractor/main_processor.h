#pragma once
#include "session.h"

namespace xr
{
  class MainProcessor {
  public:
    enum class FinderType {
      Rosenfeld,
      Radial,
      Simple
    };

    enum class GradientOpType {
      Kirsch,
      Sobel
    };

    enum Flags {
      UseOpenMP = 1,
      UseAutoBlur = 1 << 1,
      UseActiveContours = 1 << 2,
      UseAccurateSplit = 1 << 3,
    };

  private:
    int flags_ = 0;
    FinderType cont_finder_type_ = FinderType::Radial;
    GradientOpType grad_op_type_ = GradientOpType::Kirsch;
    Data::HardPtr data_;

    void prepare(uint8_t* threshold = nullptr);
    void accurateSplit(contour_t& first, contour_t& second);

  public:
    MainProcessor(int flags);
    MainProcessor(Image&& image, int flags = UseActiveContours);

    Data::HardPtr data();

    void assign(Image&& image);
    void setGradientOpType(GradientOpType type);
    void setContoursFinderType(FinderType type);

    contours_t findContours();
  };
}
