#pragma once
#include "contours_finder.h"

namespace xr
{
  class SimpleContoursFinder : public ContoursFinder {
  public:
    SimpleContoursFinder(Data::HardPtr data);

    contours_t find(Image* image, SearchMode mode, int threshold) override;
  };
}
