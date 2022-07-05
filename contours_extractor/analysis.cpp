#include <assert.h>
#include <iostream>
#include "analysis.h"
#include "utility.h"

namespace xr
{
  /* Report */
  Report::Report() :
    wtw(0),
    size(0),
    count_white_points_in_binary(0),
    used_edge_points(0),
    number_of_edge_points(0),
    covered_area(0),
    uncovered_area(0),
    covered_white_points(0),
    covered_black_points(0),
    uncovered_white_points(0),
    average_value_grad_on_edge(0.0)
  {

  }

  /* RegionInfo */
  RegionInfo::RegionInfo(int id) :
    id(id),
    size(0),
    medium_color(0),
    some_point(0, 0),
    dispersion(0),
    interest_area(false),
    standart_deviation(0),
    histogram(256),
    count_white_points_in_binary(0),
    amount_pixels_in_boundary_region(0),
    bound_rect(rect_t(Int::max(), Int::min(), Int::max(), Int::min()))
  {

  }

  bool RegionInfo::isInterestArea(uint8_t threshold) const {
    int count = 0;
    if (pixels_in_boundary_sides[0] > 0) ++count;
    if (pixels_in_boundary_sides[1] > 0) ++count;
    if (pixels_in_boundary_sides[2] > 0) ++count;
    if (pixels_in_boundary_sides[3] > 0) ++count;

    return 
      ((size - count_white_points_in_binary) * 1.0 / count_white_points_in_binary < 0.33) &&
      (medium_color>threshold);
  }

  /* other */
  std::map<int, RegionInfo> createDictionaryRegions(const regions_t& src) {
    std::map<int, RegionInfo> dictionary;
    for (size_t i = 0; i < src.size(); ++i) {
      dictionary[src[i].id] = src[i];
    }

    return dictionary;
  }

  Report createReport(const mati& marked, const Image& binary_ver, const contours_t& contours, const matd& edges, uint8_t threshold, regions_t& regs) {
    Report report;
    report.size = marked.width()*marked.height();

    // numberOfEdgePoints, averageValueGradOnEdge
    for (auto& contour : contours) {
      report.number_of_edge_points += contour.size();
      for (auto& e : contour) {
        report.average_value_grad_on_edge += edges(e);
      }
    }
    report.average_value_grad_on_edge /= (report.number_of_edge_points + 1);

    // countWhitePointsInBinary
    int nom = 0;
    for (int i = 1; i < binary_ver.width() - 1; ++i) {
      for (int j = 1; j < binary_ver.height() - 1; ++j) {
        if (!marked(i, j)) ++nom;
        if (binary_ver.byte(i, j) == 255) {
          ++report.count_white_points_in_binary;
          if (marked(i, j)) {
            ++regs[marked(i, j) - 1].count_white_points_in_binary;
          }
        }
      }
    }

    report.wtw = 1.0 - double(nom) / report.count_white_points_in_binary;

    // covered***, uncovered***
    for (auto& region : regs) {
      if (region.isInterestArea(threshold)) {
        report.covered_area += region.size;
        report.covered_black_points += region.size - region.count_white_points_in_binary;
        report.covered_white_points += region.count_white_points_in_binary;
      }
      else {
        report.uncovered_area += region.size;
        report.uncovered_white_points += region.count_white_points_in_binary;
      }
    }

    return report;
  }

  double calcMetricsQualityAllocation(const Report& report, int metric_type) {
    double denom = static_cast<double>(report.count_white_points_in_binary);
    switch (metric_type) {
    case metrics::AVERAGE_VALUE_OF_GRAD:
      return report.average_value_grad_on_edge;

    case metrics::COVERED_AREA_WP:
      return double(report.covered_white_points) / denom;

    case metrics::COVERED_AREA:
      return double(report.covered_area) / denom;

    case metrics::COVERED_AND_UNCOVERED_AREA_WP:
      return double(report.covered_white_points) / denom *
        (1.0 - double(report.covered_black_points) / (report.size - denom));
    }

    return 0;
  }

  void collectRegionInfo(const points_t& points, const Image* initial_ver, RegionInfo& region) {
    region.points = points;
    region.pixels_in_boundary_sides[0] = 0;
    region.pixels_in_boundary_sides[1] = 0;
    region.pixels_in_boundary_sides[2] = 0;
    region.pixels_in_boundary_sides[3] = 0;

    region.size = static_cast<int>(points.size());
    region.some_point = points.front();
    region.bound_rect = createBoundingRect(points);

    if (initial_ver) {
      for (auto& e : points) {
        int byte = static_cast<int>(initial_ver->byte(e));

        region.histogram[byte] += 1;
        if (e.x <= 1 || e.y <= 1 || (initial_ver->width() - e.x) <= 2 || (initial_ver->height() - e.y) <= 2) {
          ++region.amount_pixels_in_boundary_region;
        }

        if (e.x <= 1) ++region.pixels_in_boundary_sides[0];
        else if (e.y <= 1) ++region.pixels_in_boundary_sides[1];
        else if (initial_ver->width() - e.x <= 2) ++region.pixels_in_boundary_sides[2];
        else if (initial_ver->height() - e.y <= 2) ++region.pixels_in_boundary_sides[3];
      }

      double deviation = 0;
      int sum = 0, variation = 0;
      for (size_t i = 0; i<region.histogram.size(); ++i) {
        variation += static_cast<int>(i*i)*region.histogram[i];
        sum += static_cast<int>(i)*region.histogram[i];
      }

      region.medium_color = uint8_t(sum / region.size);
      region.dispersion = double(variation) / region.size - math::sqr(region.medium_color);
      for (size_t i = 0; i < region.histogram.size(); ++i) {
        deviation += region.histogram[i] * math::sqr(i - region.medium_color);
      }

      region.standart_deviation = sqrt(deviation / region.size);
    }
  }

  mati colorize(const Image& image, const Image& initial_ver, regions_t* regions) {
    int counter = 1;
    mati marked(image.size(), 0);
    for (int j = 1; j < image.height() - 1; ++j) {
      for (int i = 1; i < image.width() - 1; ++i) {
        if (image.byte(i, j) == 0 && marked(i, j) == 0) {
          auto points = image.getPointsRegion(i, j);
          for (auto &it : points) {
            marked(it) = counter; // flood fill
          }

          if (regions) {
            regions->emplace_back(counter++);
            collectRegionInfo(points, &initial_ver, regions->back());
          }
        }
      }
    }

    return marked;
  }
}