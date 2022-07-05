#include "xr_math.h"
#include <cmath>
#include <utility>

namespace xr
{
  namespace math
  {
    namespace grad
    {
      double dirInDeg(double fy, double fx) {
        return std::atan2(fy, fx) / math::Pi*180.0;
      }

      double dirInRad(double fy, double fx) {
        return std::atan2(fy, fx);
      }

      double abs(double fy, double fx) {
        return std::sqrt(fx*fx + fy*fy);
      }
    }

    double rad2deg(double rad) {
      return rad / math::Pi*180.0;
    }

    double roundDir(double angle) {
      if (angle <= 22.5 && angle > -22.5) return 0;
      if (angle > 157.5 && angle <= -157.5)  return math::Pi;
      if (angle > 22.5 && angle <= 67.5) return math::Pi_4;
      if (angle <= -112.5 && angle > -157.5) return 2 * math::Pi - math::Pi_4 * 3;
      if (angle > 67.5 && angle <= 112.5) return math::Pi_2;
      if (angle <= -67.5 && angle > -112.5) return 2 * math::Pi - math::Pi_2;
      if (angle > 112.5 && angle <= 157.5) return math::Pi_4 * 3;
      return 2 * math::Pi - math::Pi_4;
    }

    double dirDist(double f_anfle, double s_angle) {
      return math::min(abs(f_anfle - s_angle), abs(f_anfle + 2.0*math::Pi - s_angle));
    }
  }
}
