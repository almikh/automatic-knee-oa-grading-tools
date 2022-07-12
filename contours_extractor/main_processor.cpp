#include <cmath>
#include <iostream>
#include <assert.h>
#include "graph.h"
#include "utility.h"
#include "main_processor.h"
#include "active_contours.h"
#include "threshold_finder.h"
#include "multithreaded_threshold_finder.h"
#include "dev_contours_finder.h"
#include "simple_contours_finder.h"

namespace xr
{
  MainProcessor::MainProcessor(int flags) :
    grad_op_type_(GradientOpType::Kirsch),
    flags_(flags)
  {

  }

  MainProcessor::MainProcessor(Image&& image, int flags):
    grad_op_type_(GradientOpType::Kirsch),
    flags_(flags)
  {
    assign(std::move(image));
  }

  Data::HardPtr MainProcessor::data() {
    return data_;
  }

  void MainProcessor::assign(Image&& image) {
    data_.reset(new Data(std::move(image)));

    // тут будет размытие
    if (flags_ & UseAutoBlur) {
      data_->working.kuwahara(3); // TODO
      data_->working.bilateralFiltering(1.5, 1.5);
    }

    data_->prepare();
  }

  void MainProcessor::setGradientOpType(GradientOpType type) {
    grad_op_type_ = type;
  }

  void MainProcessor::setContoursFinderType(FinderType type) {
    cont_finder_type_ = type;
  }

  void MainProcessor::prepare(uint8_t* threshold) {
    Matrix<double> u, v;
    data_->working.gvf(0.0333, 70, u, v); // TODO поменьше итераций

    auto temp(std::move(matd::unite(u, v, math::grad::abs)));
    auto directon(std::move(matd::unite(v, u, math::grad::dirInDeg).transform(math::roundDir)));

    switch (grad_op_type_) {
    case GradientOpType::Sobel: data_->working.sobel(); break;
    case GradientOpType::Kirsch: data_->working.kirsch(); break;
    default:
      assert(false);
    }

    for (int i = 0; i < temp.width(); ++i) {
      for (int j = 0; j < temp.height(); ++j) {
        temp(i, j) *= data_->working.byte(i, j);
      }
    }

    data_->working.from(temp.scaled(0.0, 255.0));
    if (threshold) {
      *threshold = data_->working.thresholdByBasedGradient();
    }

    data_->working.nonMaximumSuppression(directon);
  }
  
  void MainProcessor::accurateSplit(contour_t& first, contour_t& second) {
    const double min_distance_sqr = 4.0*4.0;
    // находим проблемный участок
    points_t future_sink;
    for (auto& e1 : first) {
      for (auto& e2 : second) {
        if (e1.sqrDist(e2) < min_distance_sqr) {
          future_sink.push_back(e1);
          future_sink.push_back(e2);
        }
      }
    }

    auto roi = createBoundingRect(future_sink);
    auto inflated_roi = roi.inflated(8);

    // TODO:
    inflated_roi.bottom = xr::math::max(0, inflated_roi.bottom);
    inflated_roi.left = xr::math::max(0, inflated_roi.left);
    inflated_roi.top = xr::math::min(data_->working.height() - 1, inflated_roi.top);
    inflated_roi.right = xr::math::min(data_->working.width() - 1, inflated_roi.right);

    std::cout << roi.left << " vs " << roi.right << std::endl;
    std::cout << roi.bottom << " vs " << roi.top << std::endl;
    std::cout << "===" << std::endl;
    std::cout << inflated_roi.left << " vs " << inflated_roi.right << std::endl;
    std::cout << inflated_roi.bottom << " vs " << inflated_roi.top << std::endl;

    points_t first_sink, second_sink; // источники для поиска разреза
    first_sink.reserve(future_sink.size() / 2);
    second_sink.reserve(future_sink.size() / 2);
    point_t lb_point(inflated_roi.left, inflated_roi.bottom);
    for (auto& e : first) {
      if (inflated_roi.contains(e)) first_sink.push_back(e - lb_point);
    }

    for (auto& e : second) {
      if (inflated_roi.contains(e)) second_sink.push_back(e - lb_point);
    }

    std::set<xr::point_t> sink_coords(first_sink.begin(), first_sink.end());
    sink_coords.insert(second_sink.begin(), second_sink.end());

    auto subimage = std::move(data_->initial.subimage(inflated_roi));
    auto graph_map = std::move(subimage.to<double>());

    std::vector<int> sink, source;
    for (auto& e : sink_coords) {
      sink.push_back(e.x + e.y * inflated_roi.width());
    }

    // TODO в источник добавить еще точки МНК

    for (int i = 0; i < inflated_roi.width(); ++i) {
      source.push_back(i + 0 * inflated_roi.width());
      source.push_back(i + 1 * inflated_roi.width());
      source.push_back(i + (inflated_roi.height() - 2) * inflated_roi.width());
      source.push_back(i + (inflated_roi.height() - 1) * inflated_roi.width());
    }

    auto graph = xr::Graph::fromImage(graph_map);
    auto cut = graph.minCut(source, sink);

    points_t cut_points;
    for (auto& e : cut) {
      cut_points.emplace_back(e.first % inflated_roi.width(), e.first / inflated_roi.width());
      //cut_points.emplace_back(e.second % inflated_roi.width(), e.second / inflated_roi.width());
    }

    xr::draw(&subimage, cut_points, 255);
    std::cout << "save accurate-split.png: " << xr::imwrite(subimage, "accurate-split.png") << std::endl;
  }

  contours_t MainProcessor::findContours() {
    uint8_t threshold;
    prepare(&threshold);

    ThresholdFinder::HardPtr threshold_finder;
    if (flags_ & UseOpenMP) {
      threshold_finder = std::make_shared<MultithreadedThresholdFinder>(data_);
    }
    else {
      threshold_finder = std::make_shared<ThresholdFinder>(data_);
    }

    auto target_threshold = threshold_finder->find(threshold, 0.05, 10);

    auto item = threshold_finder->goodImage();
    data_->working = std::move(*item.image);

    contours_t contours;
    ContoursFinder::HardPtr finder;
    if (cont_finder_type_ == FinderType::Radial) {
      finder = std::make_shared<DevContoursFinder>(data_);
    }
    else {
      finder = std::make_shared<SimpleContoursFinder>(data_);
    }
    
    // ищем начальные контуры
    uint8_t otsu = data_->otsu_threshold;
    auto mode = ContoursFinder::SearchMode::FilterOut;
    contours = finder->find(&data_->working, mode, otsu);

    // далее - уточнение
    if (flags_ & UseActiveContours) {
      matd u, v;
      data_->working.gvf(0.05, 32, u, v);
      auto gvf_field = std::move(matd::unite(u, v, math::grad::abs).scaled(0, 1.0));

      ActiveContours active_contours(data_);
      active_contours.setGradientRef(&gvf_field);
      active_contours.setSimplificationDegree(5);
      //active_contours.enableUniformPointsDistribution(true);

      contours_t result;
      const int radius = 3;
      const int max_iters = 50;
      for (auto& contour : contours) {
        active_contours.run(contour, radius, max_iters);
        if (!contour.empty()) {
          result.push_back(contour);
        }
      }

      contours.swap(result);
    }

    if ((flags_ & UseAccurateSplit) && contours.size() > 1) {
      // тут будет поиск пары проблемных контуров

      auto& first = contours[0];
      auto& second = contours[1];
      accurateSplit(first, second);
    }

    return contours;
  }
}
