#pragma once
#include <map>
#include "defs.h"
#include "image.h"

namespace xr
{
  const uint8_t OBJECT_REG = 128;
  const uint8_t OBJ_BORDER_REG = 222;

  // ������� ��� ������ �������� ���������
  namespace metrics
  {
    const int AVERAGE_VALUE_OF_GRAD = 1;
    const int COVERED_AREA_WP = 2; // ������� ������� �� ����� �����
    const int COVERED_AREA = 3; // ������� ������� (��������� ��� ���. �������)
    const int COVERED_AND_UNCOVERED_AREA_WP = 4;
  }

  struct Report {
    int size;
    size_t count_white_points_in_binary;
    size_t used_edge_points; // ��������� �����, ����������� � ������� ��. �������� ������ 
    size_t number_of_edge_points; // ��������� ����� � finalVer
    size_t covered_area; // �������� �������(����� ���� - ���. ����� ������� ������)
    size_t uncovered_area; // ���������� �������
    size_t covered_white_points; // �������� ������� (������ �����)
    size_t uncovered_white_points;
    size_t covered_black_points;
    double average_value_grad_on_edge;
    double wtw;

    Report();
  };

  struct RegionInfo {
    int id;
    int size;
    rect_t bound_rect;
    point_t some_point;
    uint8_t medium_color;
    double dispersion;
    double standart_deviation;
    int amount_pixels_in_boundary_region;
    histogram_t histogram;

    bool interest_area;
    int pixels_in_boundary_sides[4];
    size_t count_white_points_in_binary;
    std::vector<point_t> points;

    RegionInfo(int id = 0);

    bool isInterestArea(uint8_t threshold) const;
  };

  using regions_t = std::vector<RegionInfo>;

  std::map<int, RegionInfo> createDictionaryRegions(const regions_t& src);

  // edges_ver - ������������ �������� ����������� (kirsch, sobel � �.�.)
  // binary_ver - �������� ������ ���������
  // final_ver - ���������� �������
  // � final_ver ������� ������� �������� ������ OBJECT_REG
  Report createReport(const mati& marked, const Image& binary_ver, const contours_t& contours, const matd& edges, uint8_t threshold, regions_t& regs);

  double calcMetricsQualityAllocation(const Report& report, int metric_type);

  void collectRegionInfo(const points_t& points, const Image* initial_ver, RegionInfo& region);

  // ����������� � �������� ������������, �� ������� - ����� ����� ������� ��������.
  // @ image - �������� ����������� � ���������� ���������.
  // @ initialVer - �������� (��������������) �����������.
  // @ regions - ������������ �������.
  // @ return - ������������ ������ (� ������ ��������), 0 - ������� ��������.
  mati colorize(const Image& image, const Image& initial_ver, regions_t* regions = nullptr);
}
