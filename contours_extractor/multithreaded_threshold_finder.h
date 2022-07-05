#pragma once
#include "threshold_finder.h"

namespace xr
{
  class MultithreadedThresholdFinder : public ThresholdFinder {
  public:
    MultithreadedThresholdFinder(Data::HardPtr data);

    uint8_t find(uint8_t approximation, double step, int count) override;
  };
}
