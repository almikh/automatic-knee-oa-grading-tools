#pragma once
#include "key_points_finder.h"
#include "session.h"

namespace xr
{
  class RadialKeyPointsFinder : public KeyPointsFinder {
    Data::HardPtr data_;

  public:
    enum ParamName {
      Points = 0 // TODO
    };

    RadialKeyPointsFinder(Data::HardPtr data);

    points_t find(const RegionInfo& region, mati* marked, const ParamsMap& params = ParamsMap()) override;
  };
}
