#include "break_points_detector.h"

namespace xr
{
  BreakPointsDetector::BreakPointsDetector(Image* pixmap) :
    target_(pixmap) 
  {

  }

  void BreakPointsDetector::setImage(Image* pixmap) {
    target_ = pixmap;
  }
}
