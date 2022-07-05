#pragma once
#include <map>
#include "defs.h"
#include "image.h"

namespace xr
{
  const uint8_t OBJECT_REG = 128;
  const uint8_t OBJ_BORDER_REG = 222;

  // метрики для оценки качества выделения
  namespace metrics
  {
    const int AVERAGE_VALUE_OF_GRAD = 1;
    const int COVERED_AREA_WP = 2; // покрыта площадь из белых точек
    const int COVERED_AREA = 3; // покрыта площадь (считается вся обл. объекта)
    const int COVERED_AND_UNCOVERED_AREA_WP = 4;
  }

  struct Report {
    int size;
    size_t count_white_points_in_binary;
    size_t used_edge_points; // граничных точек, совпадающих с точками об. бинарной версии 
    size_t number_of_edge_points; // граничных точек в finalVer
    size_t covered_area; // покрытая площадь(любой цвет - кол. точек объекта просто)
    size_t uncovered_area; // непокрытая площадь
    size_t covered_white_points; // покрытая площадь (белого цвета)
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

  // edges_ver - приближенный градиент изображения (kirsch, sobel и т.п.)
  // binary_ver - бинарная версия исходного
  // final_ver - выделенные контуры
  // В final_ver области объекта помечены цветом OBJECT_REG
  Report createReport(const mati& marked, const Image& binary_ver, const contours_t& contours, const matd& edges, uint8_t threshold, regions_t& regs);

  double calcMetricsQualityAllocation(const Report& report, int metric_type);

  void collectRegionInfo(const points_t& points, const Image* initial_ver, RegionInfo& region);

  // Применяется к бинарным изображениям, на границе - рамка цвета границы регионов.
  // @ image - бинарное изображение с замкнутыми границами.
  // @ initialVer - исходное (необработанное) изображение.
  // @ regions - получившиеся регионы.
  // @ return - раскрашенная версия (в номера регионов), 0 - границы регионов.
  mati colorize(const Image& image, const Image& initial_ver, regions_t* regions = nullptr);
}
