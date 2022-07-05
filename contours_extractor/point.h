#pragma once
#include "xr_math.h"

namespace xr
{
  template<class T>
  struct point {
    union {
      struct { T x, y; };
      T coords[2];
    };

    typedef T value_type;

    point() : x(0), y(0) {}
    point(T x, T y) : x(x), y(y) {}
    point(const point<T>& other) = default;

    template<class S>
    point(const point<S>& other) :
      x(static_cast<T>(other.x)),
      y(static_cast<T>(other.y)) {

    }

    point& operator=(const point<T>& rhs) {
      if (this == &rhs) return *this;

      x = rhs.x;
      y = rhs.y;

      return *this;
    }

    template<class S>
    point& operator=(const point<S>& rhs) {
      x = T(rhs.x);
      y = T(rhs.y);

      return *this;
    }

    point<T>& operator/=(const T& rhs) {
      x /= rhs;
      y /= rhs;
      return *this;
    }

    point<T>& operator*=(const T& rhs) {
      x *= rhs;
      y *= rhs;
      return *this;
    }

    point<T>& operator+=(const T& rhs) {
      x += rhs;
      y += rhs;
      return *this;
    }

    point<T>& operator+=(const point<T>& rhs) {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    point<T>& operator-=(const T& rhs) {
      x -= rhs;
      y -= rhs;
      return *this;
    }

    point<T>& operator-=(const point<T>& rhs) {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    point<T>& normalize() {
      static_assert(std::is_floating_point<T>::value, "Floating point required.");

      auto length = std::sqrt(x*x + y*y);
      x /= length;
      y /= length;
      return *this;
    }

    point<T> normalized() const {
      static_assert(std::is_floating_point<T>::value, "Floating point required.");

      point<T> dst(*this);
      return dst.normalize();
    }

    point& swap() {
      T temp = x;
      x = y;
      y = temp;
      return *this;
    }

    template<class S>
    double dist(const point<S>& other) const {
      return std::sqrt(math::sqr(x - other.x) + math::sqr(y - other.y));
    }

    template<class S>
    double sqrDist(const point<S>& other) const {
      return math::sqr(x - other.x) + math::sqr(y - other.y);
    }

    template<class S>
    point<S> to() {
      return point<S>(static_cast<S>(x), static_cast<S>(y));
    }
  };

  using point_t = point<int>;

  template<class T>
  point<T> make_point(const T& x, const T& y) {
    return point<T>(x, y);
  }

  template<class T>
  double dist(const point<T>& p1, const point<T>& p2) {
    return p1.dist(p2);
  }

  template<class T>
  double sqrDist(const point<T>& p1, const point<T>& p2) {
    return math::sqr(p1.x - p2.x) + math::sqr(p1.x - p2.y);
  }

  template<class T>
  inline point<T> operator-(const point<T>& lhs, const point<T>& rhs) {
    return point<T>(lhs.x - rhs.x, lhs.y - rhs.y);
  }

  template<class T>
  inline point<T> operator+(const point<T>& lhs, const point<T>& rhs) {
    return point<T>(lhs.x + rhs.x, lhs.y + rhs.y);
  }

  template<class T>
  inline bool operator==(const point<T>& lhs, const point<T>& rhs) {
    return (lhs.x == rhs.x && lhs.y == rhs.y);
  }

  template<class T>
  inline bool operator!=(const point<T>& lhs, const point<T>& rhs) {
    return (!(lhs == rhs));
  }

  template<class T>
  inline bool operator<(const point<T>& lhs, const point<T>& rhs) {
    return (lhs.x < rhs.x || (!(rhs.x < lhs.x) && lhs.y < rhs.y));
  }

  template<class T>
  inline bool operator>(const point<T>& lhs, const point<T>& rhs) {
    return (rhs < lhs);
  }

  template<class T>
  inline bool operator<=(const point<T>& lhs, const point<T>& rhs) {
    return (!(rhs < lhs));
  }

  template<class T>
  inline bool operator>=(const point<T>& lhs, const point<T>& rhs) {
    return (!(lhs < rhs));
  }
}
