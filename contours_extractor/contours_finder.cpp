#include "contours_finder.h"
#include "utility.h"
#include <assert.h>

namespace xr
{
  ContoursFinder::ContoursFinder(Data::HardPtr data):
    data_(data)
  {

  }

  ContoursFinder::~ContoursFinder() {
  
  }

  contour_t ContoursFinder::initialContour(const points_t& key_points) {
    assert(path_finder_ != nullptr);

    contour_t result = key_points;
    int flags = PathFinder::Distance | PathFinder::Gradient | PathFinder::GradientDiff;
    return amplify(result, path_finder_.get(), flags);
  }
}
