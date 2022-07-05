#pragma once
#include "contours_finder.h"

namespace xr
{
  class DevContoursFinder : public ContoursFinder {
  public:
    DevContoursFinder(Data::HardPtr data);

    contours_t find(Image* image, SearchMode mode, int threshold) override;
  };
}
