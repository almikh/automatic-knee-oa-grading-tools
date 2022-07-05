#pragma once
#include "point.h"
#include "image.h"

namespace xr {
  class BreakPointsDetector {
    Image* target_ = nullptr;

  public:
    explicit BreakPointsDetector(Image* target = nullptr);

    void setImage(Image* image);

    // TODO упростить if'ы уменьшить кол-во проверок
    bool is1stType(int x, int y) const {
      if (0 == x || x == target_->width() - 1 || 0 == y || y == target_->height() - 1) {
        return false;
      }

      int count = 0;
      if (target_->byte(x - 1, y - 1) == 255) ++count;
      if (target_->byte(x + 0, y - 1) == 255) ++count;
      if (target_->byte(x + 1, y - 1) == 255) ++count;
      if (target_->byte(x - 1, y + 0) == 255) ++count;
      if (target_->byte(x + 1, y + 0) == 255) ++count;
      if (target_->byte(x - 1, y + 1) == 255) ++count;
      if (target_->byte(x + 0, y + 1) == 255) ++count;
      if (target_->byte(x + 1, y + 1) == 255) ++count;

      return (count <= 1);
    }

    bool is2ndType(int x, int y) const {
      if (0 == x || x == target_->width() - 1 || 0 == y || y == target_->height() - 1) {
        return false;
      }

      if (!target_->byte(x - 1, y) && !target_->byte(x - 1, y - 1) && !target_->byte(x, y - 1) && !target_->byte(x + 1, y - 1) && !target_->byte(x + 1, y) && (!target_->byte(x - 1, y + 1) || !target_->byte(x + 1, y + 1))) return true;
      if (!target_->byte(x, y - 1) && !target_->byte(x + 1, y - 1) && !target_->byte(x + 1, y) && !target_->byte(x + 1, y + 1) && !target_->byte(x, y + 1) && (!target_->byte(x - 1, y - 1) || !target_->byte(x - 1, y + 1))) return true;
      if (!target_->byte(x + 1, y) && !target_->byte(x + 1, y + 1) && !target_->byte(x, y + 1) && !target_->byte(x - 1, y + 1) && !target_->byte(x - 1, y) && (!target_->byte(x - 1, y - 1) || !target_->byte(x + 1, y - 1))) return true;
      if (!target_->byte(x, y + 1) && !target_->byte(x - 1, y + 1) && !target_->byte(x - 1, y) && !target_->byte(x - 1, y - 1) && !target_->byte(x, y - 1) && (!target_->byte(x + 1, y - 1) || !target_->byte(x + 1, y + 1))) return true;
      return false;
    }

    bool is3rdType(int x, int y) const {
      if (0 == x || x == target_->width() - 1 || 0 == y || y == target_->height() - 1) {
        return false;
      }

      if (!target_->byte(x - 1, y + 1) && !target_->byte(x - 1, y) && !target_->byte(x - 1, y - 1) && !target_->byte(x, y - 1) && !target_->byte(x + 1, y - 1)) return true;
      else if (!target_->byte(x - 1, y - 1) && !target_->byte(x, y - 1) && !target_->byte(x + 1, y - 1) && !target_->byte(x + 1, y) && !target_->byte(x + 1, y + 1)) return true;
      else if (!target_->byte(x + 1, y - 1) && !target_->byte(x + 1, y) && !target_->byte(x + 1, y + 1) && !target_->byte(x, y + 1) && !target_->byte(x - 1, y + 1)) return true;
      else if (!target_->byte(x + 1, y + 1) && !target_->byte(x, y + 1) && !target_->byte(x - 1, y + 1) && !target_->byte(x - 1, y) && !target_->byte(x - 1, y - 1)) return true;
      return false;
    }

    bool is1st2ndType(int x, int y) const {
      return is1stType(x, y) || is2ndType(x, y);
    }

    bool is1st3rdType(int x, int y) const {
      return is1stType(x, y) || is3rdType(x, y);
    }

    bool is2nd3rdType(int x, int y) const {
      return is3rdType(x, y) || is2ndType(x, y);
    }

    bool is1st2nd3rdType(int x, int y) const {
      return is1stType(x, y) || is2ndType(x, y) || is3rdType(x, y);
    }

    bool is1stType(const point_t& point) const {
      return is1stType(point.x, point.y);
    }

    bool is2ndType(const point_t& point) const {
      return is2ndType(point.x, point.y);
    }

    bool is3rdType(const point_t& point) const {
      return is3rdType(point.x, point.y);
    }

    bool is1st2ndType(const point_t& point) const {
      return is1stType(point.x, point.y) || is2ndType(point.x, point.y);
    }

    bool is1st3rdType(const point_t& point) const {
      return is1stType(point.x, point.y) || is3rdType(point.x, point.y);
    }

    bool is2nd3rdType(const point_t& point) const {
      return is3rdType(point.x, point.y) || is2ndType(point.x, point.y);
    }

    //bool is1st2nd3rdType(const point_t& point) const {
    //  return is1stType(point.x, point.y) || is2ndType(point.x, point.y) || is3rdType(point.x, point.y);
    //}
  };
}
