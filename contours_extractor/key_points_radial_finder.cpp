#include "key_points_radial_finder.h"
#include "utility.h"
#include <iostream>

namespace xr
{
  RadialKeyPointsFinder::RadialKeyPointsFinder(Data::HardPtr data) :
    data_(data)
  {

  }

  points_t RadialKeyPointsFinder::find(const RegionInfo& region, mati* marked, const ParamsMap& params) {
    double temp;
    point_t center = region.points.front(); // точка региона, ближе всего расположенная к его центру
    auto real_center = region.bound_rect.center();
    double temp_dist = real_center.dist(center);
    for (auto& p : region.points) {
      temp = real_center.dist(p);
      if (temp < temp_dist) {
        temp_dist = temp;
        center = p;
      }
    }

    int w = marked->width(), h = marked->height();
    size_t full_range = w * 2 + h * 2;
    size_t points_num = params.at(ParamName::Points).ivalue;
    int step = full_range / points_num, cur = 0;
    points_t bound_points; // крайние точки (на основе их будем искать ключевые)
    while (bound_points.size() < points_num) {
      if (cur < h) { // левая сторона изображения
        bound_points.emplace_back(0, cur);
      }
      else if (cur < h + w) { // верх изображения
        bound_points.emplace_back(cur - h, h - 1); 
      }
      else if (cur < h + w + h) { // правая часть изображения
        bound_points.emplace_back(w - 1, h - (cur - h - w) - 1);
      }
      else { // нижняя часть
        bound_points.emplace_back(w - (cur - h - w - h), 0);
      }

      cur += step;
    }

    // трассируем путь от центра до точки в описках подходящей границы
    points_t key_points;
    int main_label = marked->at(center);
    for (auto& point : bound_points) {
      point_t key_point;
      bool found = false;
      auto path = makeDirectPath(center, point);
      for (size_t i = 1; i < path.size(); ++i) {
        if (!marked->isCorrect(path[i])) continue;

        auto label = marked->at(path[i]);
        if (found && label && label != main_label && !marked->at(path[i - 1])) {
          key_point = path[i - 1];
        }

        if (label) found = (label == main_label);
      }

      if (found && !marked->at(path.back())) {
        key_point = path.back();
      }

      if (key_point != point_t()) {
        key_points.push_back(key_point);
      }
    }

    return key_points;
  }
}
