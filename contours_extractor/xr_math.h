#pragma once

namespace xr
{
  namespace math
  {
    static const double Pi = 3.14159265358979323846;
    static const double Pi_2 = 1.57079632679489661923;
    static const double Pi_4 = 0.78539816339744830962;

    // обход крест-накрест
    const int dx[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
    const int dy[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };

    // обход соседей по часовой стрелке
    const int cdx[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
    const int cdy[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };

    template<class T>
    inline T min(T a, T b) {
      return (a < b) ? a : b;
    }

    template<class T>
    inline T max(T a, T b) {
      return (a > b) ? a : b;
    }

    template<class T>
    inline T sqr(T value) {
      return value*value;
    }

    template<class T>
    inline int sign(const T& val) {
      //static_assert(std::is_integral<T>::value, "Integer required.");

      if (val == 0) return 0;
      return (val < 0) ? -1 : 1;
    }

    template<class T>
    inline T normalize(T val, T step) {
      if (val < 0) return val + step;
      else if (val >= step) return val - step;
      else return val;
    }

    // используется для преобразования градиента в скаляр
    namespace grad
    {
      // направление в градусах
      double dirInDeg(double fy, double fx);

      // направление в радианах
      double dirInRad(double fy, double fx);

      // модуль
      double abs(double fy, double fx);
    }

    // переводит радианы в градусы
    double rad2deg(double rad);

    // округляет направление до ближайшего 45-градусного деления
    double roundDir(double angle);

    // расстояние между направлениями (в радианах)
    double dirDist(double angle1, double angle2);
  }
}
