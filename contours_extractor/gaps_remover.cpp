#include <set>
#include <list>
#include <iostream>
#include "gaps_remover.h"
#include "image_info.h"
#include "xr_math.h"
#include "utility.h"

namespace xr
{
  GapsRemover::GapsRemover(Data::HardPtr data, Image* target_image) :
    data_(data),
    path_finder_(new PathFinder(data->initial.size())),
    target_image_(target_image)
  {
    // path_finder_->setGradientRef(&data_->gradient);
    path_finder_->setGradient(data_->gradient);
    detector_.setImage(target_image_);
  }

  bool GapsRemover::isFormedSmallAreas(const path_t& path) {
    path_t neighbors;
    point_t point(-1, -1);
    for (size_t i = 0; i<path.size(); ++i) {
      if (info::neighborsNumber(*target_image_, path[i].x, path[i].y) == 2) {
        neighbors = info::neighborsPixel(*target_image_, path[i].x, path[i].y);
        if (neighbors.front().dist(neighbors.back()) >= 2) {
          point = path[i];
          break;
        }
      }
    }

    if (point.x == -1) return false;

    auto inds = info::neighborsPixelIndices(*target_image_, point.x, point.y);
    int x = point.x;
    int y = point.y;
    int i1 = math::normalize(inds.front() + 1, 8);
    int i2 = math::normalize(inds.back() + 1, 8);
    size_t limit = target_image_->width()*target_image_->height() / 100;

    return 
      target_image_->getPointsRegion(x + math::dx[i1], y + math::dy[i1], Connectivity::Four, limit + 2).size() < limit || 
      target_image_->getPointsRegion(x + math::dx[i2], y + math::dy[i2], Connectivity::Four, limit + 2).size() < limit;
  }
  
  PathFinder::HardPtr GapsRemover::pathFinder() {
    return path_finder_;
  }

  void GapsRemover::assign(Data::HardPtr data, Image* target_image) {
    data_ = data;
    target_image_ = target_image;
    detector_.setImage(target_image_);
    path_finder_.reset(new PathFinder(data->initial.size()));
    // path_finder_->setGradientRef(&data_->gradient); 
    path_finder_->setGradient(data_->gradient);
  }

  void GapsRemover::setMaxLength(int max_length) {
    max_length_ = max_length;
  }

  void GapsRemover::runMain(IsBreak is_gap) {
    int removed = 0;
    points_t points; // точки разрыва
    for (int j = 1; j<target_image_->height() - 1; ++j) {
      for (int i = 1; i<target_image_->width() - 1; ++i) {
        if (target_image_->byte(i, j) == 255 && (detector_.*is_gap)(i, j))
          points.emplace_back(i, j);
      }
    }

    auto test = [&](const point_t& p, const point_t& q) -> bool {
      auto neighbors = info::neighborsPixel(*target_image_, p.x, p.y);
      if (neighbors.empty()) return true;

      const double d = math::Pi_2;

      auto r = neighbors.front();
      auto t = (q - p).to<double>().normalize();
      auto u = (p - r).to<double>().normalize();
      double v_x = u.x * cos(-d) - u.y * sin(-d);
      double v_y = u.x * sin(-d) + u.y * cos(-d);
      double w_x = u.x * cos(d) - u.y * sin(d);
      double w_y = u.x * sin(d) + u.y * cos(d);

      // TODO что-то тут упущено
      return true; //  (w_x*t.y - w_y*t.x >= 0) && (t.x*v_y - t.y*v_x < 0);
    };

    int n = static_cast<int>(points.size());
    // TODO память можно в два раза уменьшить (матрица то треугольная получается)
    // TODO не рассматривать соседние точки разрыва
    matd distances(n, n, Double::max()); // матрица расстояний
    for (int i = 0; i < n; ++i) {
      for (int j = i + 1; j < n; ++j) {
        auto d = points[i].dist(points[j]);
        if (d >= 2 && (test(points[i], points[j]) || test(points[j], points[i]))) {
          distances(i, j) = distances(j, i) = d;
        }
      }
    }

    double min_dist;
    int f_point, s_point;
    auto size = points.size();
    for (size_t cur = 0; cur<size; ++cur) {
      f_point = s_point = 0;
      min_dist = Double::max();
      for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
          if (distances(i, j) < min_dist) {
            min_dist = distances(i, j);
            f_point = i;
            s_point = j;
          }
        }
      }

      if (min_dist == Double::max() || min_dist > max_length_) {
        break;
      }

      int flags = PathFinder::Distance | PathFinder::Gradient;
      path_finder_->find(points[f_point], points[s_point], flags, false);

      // TODO
      // auto path = xr::makeDirectPath(points[f_point], points[s_point]);

      auto path = path_finder_->lastPath();
      int path_size = static_cast<int>(path.size());
      if (/*1 < path_size && */path_size <= max_length_) {
        int i, n = path.size() - 1;
        for (i = 1; i < n && target_image_->byte(path[i]) != 255; ++i);

        if (i >= n) { // не пересекает другую границу
          for (size_t i = 0, nn = path.size(); i < nn; ++i) target_image_->byte(path[i]) = 255;
          bool fl = isFormedSmallAreas(path);
          if (fl) {
            for (size_t i = 1; i < path.size() - 1; ++i) target_image_->byte(path[i]) = 0;
            ++removed;
          }
        }
      }

      distances(f_point, s_point) = Double::max();
      distances(s_point, f_point) = Double::max();
      if (!(detector_.*is_gap)(points[f_point].x, points[f_point].y)) {
        for (size_t i = 0; i < size; ++i) {
          distances(f_point, i) = distances(i, f_point) = Double::max();
        }
      }

      if (!(detector_.*is_gap)(points[s_point].x, points[s_point].y)) {
        for (size_t i = 0; i < size; ++i) {
          distances(s_point, i) = distances(i, s_point) = Double::max();
        }
      }
    }
  }

  void GapsRemover::runAuxiliary(Predicat predicat) {
    int removed = 0; // сколько разрывов было устранено (для статистики)
    points_t points; // точки разрыва
    for (int j = 1; j < data_->initial.height() - 1; ++j) {
      for (int i = 1; i < data_->initial.width() - 1; ++i) {
        if (target_image_->byte(i, j) == 255 && detector_.is1st2nd3rdType(i, j)) {
          points.emplace_back(i, j);
        }
      }
    }

    double price = 0;
    std::set<point_t> pts;
    double medium = data_->gradient.medium();
    for (auto& p : points) {
      path_t path = { p };
      for (auto& e : info::neighborsPixelIndices(*target_image_, path.back().x, path.back().y)) {
        pts.emplace(path.back().x + math::dx[e], path.back().y + math::dy[e]);
      }

      bool flag = true;
      do {
        auto last = path.back();
        auto dir = math::roundDir(math::rad2deg(data_->gradient_dir(last)));
        int dx = math::sign(static_cast<int>(round(cos(dir))));
        int dy = math::sign(static_cast<int>(round(sin(dir))));
        if (!pts.count(point_t(last.x + dx, last.y + dy))) {
          path.push_back(point_t(last.x + dx, last.y + dy));
          pts.insert(last);
        }
        else if (!pts.count(point_t(last.x - dx, last.y - dy))) {
          path.push_back(point_t(last.x - dx, last.y - dy));
          pts.insert(last);
        }
        else {
          flag = false;
          break;
        }
      } while (!predicat(target_image_, path.back().x, path.back().y) && static_cast<int>(path.size()) < max_length_);

      price = 0;
      for (auto& e : path) {
        price += data_->gradient_dir(e);
      }

      int path_size = static_cast<int>(path.size());
      if (flag && path_size<max_length_ && price>path_size*medium*factor_) {
        path_t free_points; // тут только новые точки, не принадлежащие раньше границе
        free_points.reserve(path.size());
        for (size_t i = 0; i < path.size(); ++i) {
          if (target_image_->byte(path[i]) != 255) {
            free_points.push_back(path[i]);
          }
        }

        if (free_points.size() == path.size()) {
          ++removed;
          // отмечаем на изображении
          for (auto& e : free_points) {
            target_image_->byte(e) = 255;
          }

          // если есть мелкие области - возвращаем обратно
          if (isFormedSmallAreas(path)) {
            --removed;
            for (auto& e : free_points) {
              target_image_->byte(e) = 0;
            }
          }
        }
      }
    }
  }
}
