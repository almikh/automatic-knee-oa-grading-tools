#pragma once
#include <map>
#include "defs.h"
#include "point.h"
#include "analysis.h"

namespace xr
{
  union ParamValue {
    int ivalue;
    double dvalue;
    void* pointer;
  };

  using ParamsMap = std::map<int, ParamValue>;

  class KeyPointsFinder {
  public:
    virtual ~KeyPointsFinder() {}

    virtual points_t find(const RegionInfo& region, mati* marked, const ParamsMap& params) = 0;
  };
}
