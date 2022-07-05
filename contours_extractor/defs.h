#pragma once
#include <stdint.h>
#include <vector>
#include <limits>

#include "point.h"

using Int = std::numeric_limits<int>;
using Float = std::numeric_limits<float>;
using Double = std::numeric_limits<double>;

template<class T>
class Type : public std::numeric_limits<T> {};

namespace xr
{
  enum class Orientation {
    Clockwise,
    AntiClockwise
  };

  enum Connectivity {
    Four = 4,
    Eight = 8
  };

  typedef std::vector<point_t> path_t;
  typedef std::vector<point_t> points_t;
  typedef std::vector<point_t> contour_t;
  typedef std::vector<int> histogram_t;
  typedef std::vector<contour_t> contours_t;
}
