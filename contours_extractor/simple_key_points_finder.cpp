#include "simple_key_points_finder.h"

namespace xr
{
  SimpleKeyPointsFinder::SimpleKeyPointsFinder(Data::HardPtr data) :
    data_(data) 
  {

  }

  points_t SimpleKeyPointsFinder::keyPointsOfRegion_left(const RegionInfo& region, const mati& marked, point_t shift) {
    points_t left;
    int step = (region.bound_rect.top - region.bound_rect.bottom) / 5 + 1;
    for (int j = region.bound_rect.bottom + shift.x + step; j < region.bound_rect.top - shift.y; j += step) {
      int x = region.bound_rect.left;
      while (marked(x, j) != region.id) ++x;
      --x;

      // все, стартовая точка области -> (x, j)
      int views = -1;
      double init_dir = data_->gradient_dir(x, j);
      while (marked.isCorrect(x + views, j) && marked(x + views, j) == 255) {
        if (data_->gradient(x + views, j) > data_->gradient(x, j) && math::dirDist(init_dir, data_->gradient_dir(x + views, j)) < math::Pi_2) {
          x += views;
          views = 0;
        }

        --views;
      }

      left.push_back(point_t(x, j));
    }

    return left;
  }

  points_t SimpleKeyPointsFinder::keyPointsOfRegion_right(const RegionInfo& region, const mati& marked, point_t shift) {
    points_t right;
    int step = (region.bound_rect.top - region.bound_rect.bottom) / 5 + 1;
    for (int j = region.bound_rect.bottom + shift.x + step; j < region.bound_rect.top - shift.y; j += step) {
      int x = region.bound_rect.right;
      while (marked(x, j) != region.id) --x;
      ++x;

      // все, стартовая точка области -> (x, j)
      int views = 1;
      double init_dir = data_->gradient_dir(x, j);
      while (marked.isCorrect(x + views, j) && marked(x + views, j) == 255) {
        if (data_->gradient(x + views, j) > data_->gradient(x, j) && math::dirDist(init_dir, data_->gradient_dir(x + views, j)) < math::Pi_2) {
          x += views;
          views = 0;
        }

        ++views;
      }

      right.push_back(point_t(x, j));
    }

    return right;
  }

  points_t SimpleKeyPointsFinder::keyPointsOfRegion_top(const RegionInfo& region, const mati& marked, point_t shift) {
    points_t top;
    int step = (region.bound_rect.right - region.bound_rect.left) / 5 + 1;
    for (int i = region.bound_rect.left + shift.x + step; i < region.bound_rect.right - shift.y; i += step) {
      int y = region.bound_rect.bottom;
      while (marked(i, y) != region.id) ++y;
      --y;

      // все, стартовая точка области -> (i, y)
      int views = -1;
      double init_dir = data_->gradient_dir(i, y);
      while (marked.isCorrect(i, y + views) && marked(i, y + views) == 255) {
        if (data_->gradient(i, y + views) > data_->gradient(i, y) && math::dirDist(init_dir, data_->gradient_dir(i, y + views)) < math::Pi_2) {
          y += views;
          views = 0;
        }

        --views;
      }

      top.push_back(point_t(i, y));
    }

    return top;
  }

  points_t SimpleKeyPointsFinder::keyPointsOfRegion_bottom(const RegionInfo& region, const mati& marked, point_t shift) {
    points_t bottom;
    int step = (region.bound_rect.right - region.bound_rect.left) / 5 + 1;
    for (int i = region.bound_rect.left + shift.x + step; i < region.bound_rect.right - shift.y; i += step) {
      int y = region.bound_rect.top;
      while (marked(i, y) != region.id) --y;
      ++y;

      // все, стартовая точка области -> (i, y)
      int views = 1;
      double init_dir = data_->gradient_dir(i, y);
      while (marked.isCorrect(i, y + views) && marked(i, y + views) == 255) {
        if (data_->gradient(i, y + views) > data_->gradient(i, y) && math::dirDist(init_dir, data_->gradient_dir(i, y + views)) < math::Pi_2) {
          y += views;
          views = 0;
        }

        ++views;
      }

      bottom.push_back(point_t(i, y));
    }

    return bottom;
  }

  points_t SimpleKeyPointsFinder::find(const RegionInfo& region, mati* marked, const ParamsMap& params) {
    int width = region.bound_rect.width();
    int height = region.bound_rect.height();
    points_t top, bottom, left, right;
    if (width > height) {
      top = keyPointsOfRegion_top(region, *marked, point_t(0, 0));
      bottom = keyPointsOfRegion_bottom(region, *marked, point_t(0, 0));

      point_t rt = top.empty() ? point_t(0, 0) : top.front();
      point_t rb = bottom.empty() ? point_t(0, 0) : bottom.front();
      point_t lt = rt, lb = rb;
      for (auto &it : top) {
        if (lt.x > it.x) lt = it;
        if (rt.x < it.x) rt = it;
      }

      for (auto &it : bottom) {
        if (lb.x > it.x) lb = it;
        if (rb.x < it.x) rb = it;
      }

      point_t left_shift(abs(lt.y - region.bound_rect.bottom), abs(lb.y - region.bound_rect.top));
      point_t right_shift(abs(rt.y - region.bound_rect.bottom), abs(rb.y - region.bound_rect.top));
      keyPointsOfRegion_left(region, *marked, left_shift);
      keyPointsOfRegion_right(region, *marked, right_shift);
    }
    else {
      left = keyPointsOfRegion_left(region, *marked, point_t(0, 0));
      right = keyPointsOfRegion_right(region, *marked, point_t(0, 0));

      point_t rt = right.empty() ? point_t(0, 0) : right.front();
      point_t lt = left.empty() ? point_t(0, 0) : left.front();
      point_t rb = rt, lb = lt;
      for (auto &it : left) {
        if (lt.y > it.y) lt = it;
        if (lb.y < it.y) lb = it;
      }
      for (auto &it : right) {
        if (rt.y > it.y) rt = it;
        if (rb.y < it.y) rb = it;
      }

      point_t top_shift(abs(lt.x - region.bound_rect.left), abs(rt.x - region.bound_rect.right));
      point_t bottom_shift(abs(lb.x - region.bound_rect.left), abs(rb.x - region.bound_rect.right));
      top = keyPointsOfRegion_top(region, *marked, top_shift);
      bottom = keyPointsOfRegion_bottom(region, *marked, bottom_shift);
    }

    top.insert(top.end(), right.begin(), right.end());
    top.insert(top.end(), bottom.rbegin(), bottom.rend());
    top.insert(top.end(), left.rbegin(), left.rend());

    return top;
  }
}
