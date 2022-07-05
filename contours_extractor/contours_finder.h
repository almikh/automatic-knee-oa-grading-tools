#pragma once
#include "session.h"
#include "path_finder.h"

namespace xr
{
  class ContoursFinder {
  public:
    using HardPtr = std::shared_ptr<ContoursFinder>;

  protected:
    Data::HardPtr data_;
    PathFinder::HardPtr path_finder_;

    contour_t ContoursFinder::initialContour(const points_t& key_points);

  public:
    enum class SearchMode {
      All,
      FilterOut
    };

    ContoursFinder(Data::HardPtr data);
    ~ContoursFinder();

    virtual contours_t find(Image* image, SearchMode mode, int threshold) = 0;
  };
}
