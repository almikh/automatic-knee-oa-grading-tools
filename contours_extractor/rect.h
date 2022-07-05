#pragma once
#include "point.h"

namespace xr
{
  template<class T>
  struct rect {
    T left, top, right, bottom;

    rect() : left(0), top(0), right(0), bottom(0) {}

    rect(T left, T right, T bottom, T top) :
      left(left),
      top(top),
      right(right),
      bottom(bottom) 
    {

    }

    T width() const {
      return abs(left - right);
    }

    T height() const {
      return abs(bottom - top);
    }

    bool contains_eq(T x, T y) const {
      return (x >= left) && (y >= bottom) && (x <= right) && (y <= top);
    }

    bool contains_eq(const point<T>& pt) const {
      return contains_eq(pt.x, pt.y);
    }

    bool contains(T x, T y) const {
      return x > left && y > bottom && x < right && y < top;
    }

    bool contains(const point<T>& pt) const {
      return contains(pt.x, pt.y);
    }

    bool operator==(const rect<T> other) {
      return left == other.left && right == other.right && top == other.top && bottom == other.bottom;
    }

    bool operator!=(const rect<T> other) {
      return !(other == *this);
    }

    point<T> center() const {
      return make_point((left + right) / 2, (top + bottom) / 2);
    }

    rect& inflate(int margin) {
      left -= margin;
      right += margin;
      bottom -= margin;
      top += margin;
      return *this;
    }

    rect inflated(int margin) const {
      return rect(*this).inflate(margin);
    }
  };

  using rect_t = rect<int>;
}
