#pragma once
#include "session.h"
#include "path_finder.h"
#include "break_points_detector.h"

namespace xr
{
  using Predicat = bool(*)(Image*, int, int);
  using IsBreak = bool(BreakPointsDetector::*)(int, int) const;

  class GapsRemover {
    Data::HardPtr data_;
    BreakPointsDetector detector_;
    PathFinder::HardPtr path_finder_;
    Image* target_image_;

    int max_length_ = Int::max();
    double factor_ = 1.0;

    bool isFormedSmallAreas(const path_t& path);

  public:
    GapsRemover() = default;
    GapsRemover(Data::HardPtr data, Image* target_image);

    PathFinder::HardPtr pathFinder();

    void assign(Data::HardPtr data, Image* target_image);
    void setMaxLength(int max_length);

    virtual void runMain(IsBreak isGap);
    virtual void runAuxiliary(Predicat predicat);
  };
}
