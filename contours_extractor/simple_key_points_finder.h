#pragma once
#include "key_points_finder.h"
#include "session.h"

namespace xr
{
  class SimpleKeyPointsFinder : public KeyPointsFinder {
    Data::HardPtr data_;

    points_t keyPointsOfRegion_left(const RegionInfo& region, const mati& marked, point_t shift);
    points_t keyPointsOfRegion_right(const RegionInfo& region, const mati& marked, point_t shift);
    points_t keyPointsOfRegion_top(const RegionInfo& region, const mati& marked, point_t shift);
    points_t keyPointsOfRegion_bottom(const RegionInfo& region, const mati& marked, point_t shift);
    points_t keyPointsOfRegion(const RegionInfo& region, const mati& marked);

  public:
    enum ParamName {
      Points = 0
    };

    SimpleKeyPointsFinder(Data::HardPtr data);

    points_t find(const RegionInfo& region, mati* marked, const ParamsMap& params = ParamsMap()) override;
  };
}
