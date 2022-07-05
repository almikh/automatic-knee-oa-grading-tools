#pragma once
#include <algorithm>
#include <assert.h>
#include "defs.h"
#include "rect.h"
#include "image.h"
#include "path_finder.h"

namespace xr
{
  class PathFinder;
  // ���������� �������������� ������ �������������
  rect_t createBoundingRect(const contour_t& src);

  // ���������� ���������� �������
  Orientation orientation(const contour_t& src);

  // ����������� ������ �� �������� ����������� ���������� �����������
  contour_t getContourByRosenfeld(const Image& image);

  // �������� ������ � ���������, ��������������� ��������������� ��������� ��������
  contour_t toFundamental(const contour_t& src);

  contour_t toFundamental(const Image& src);

  // ����������� ������ (��������� ���� ������ k-�� �����); ������������ ������� ������
  contour_t& simplify(contour_t& src, int k);

  // ��������� ����������� ������-������ �������, ����� ���������� ����� ��������� ���� �� ������ sqrt(2)
  contour_t& amplify(contour_t& src, PathFinder* path_finder, int flags = PathFinder::Distance | PathFinder::Gradient);

  // ���������� ����� �����, ������� ������ ������ ����� �� start � finish �� ������
  points_t makeDirectPath(point_t start, point_t finish);

  // ������� ������������� �������� �������
  template<typename T> 
  std::vector<T>& unique(std::vector<T>& src) {
    return src;

    // TODO
    size_t n = src.size();
    int size = 0, shift = 0;
    for (size_t i = 0; i < n; ++i) {
      for (size_t j = 0; j < n; ++j) {
      
      }
    }

    /*for (auto &e : src) {
      bool fl = true;
      for (auto &e2 : dst) {
        if (e == e2) {
          fl = false;
          break;
        }
      }
      if (fl) dst.push_back(e);
    }*/

    assert(false);
    return src;
  }

  double rmsError(const contour_t& first, const contour_t& other);

  double meanDistance(const contour_t& first, const contour_t& other);

  double standartDeviation(const contour_t& first, const contour_t& other);

  double hausdorfDistance(const contour_t& first, const contour_t& other);
}
