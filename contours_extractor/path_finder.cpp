#include "path_finder.h"
#include <assert.h>
#include <iostream>
#include <list>
#include <map>

namespace xr
{
  PathFinder::PathFinder() {

  }

  PathFinder::PathFinder(const Size& size) :
    pass_map_(new AllTraversable()),
    aux_price_(size, 0),
    label_map_(size, 0),
    price_(size, 0) 
  {

  }

  bool PathFinder::find(point_t first, point_t last, int flag, bool include_init_points) {
    assert(pass_map_ != nullptr);

    last_path_.clear();
    if (first == last) return true;

    std::list<point_t> open(1, first);
    std::map<point_t, point_t> parents;
    label_map_(first) = ++label_;

    double min_val, temp;
    point_t cur, temp_cur;
    std::list<point_t>::iterator min_ind;
    for (;;) {
      if (open.empty()) return false;

      min_val = Double::max();
      for (auto it = open.begin(); it != open.end(); ++it) {
        if (min_val > aux_price_(*it)) {
          min_val = aux_price_(*it);
          min_ind = it;
        }
      }

      cur = *min_ind;
      if (cur == last) break;
      open.erase(min_ind);

      for (int i = 0; i<8; ++i) {
        temp_cur = point_t(cur.x + math::dx[i], cur.y + math::dy[i]);
        if (label_map_.isCorrect(temp_cur)) {
          if (label_map_(temp_cur) != label_ && pass_map_->isTraversable(temp_cur)) {
            price_(temp_cur) = price_(cur) + dist(temp_cur, cur);
            label_map_(temp_cur) = label_;
            parents.emplace(temp_cur, cur);
            open.push_back(temp_cur);

            temp = price_(temp_cur);     
            if (flag & Gradient) temp += -grad_ref_->at(temp_cur);
            if (flag & Distance) temp += dist(temp_cur, last);
            if (flag & GradientDiff) temp += abs(grad_ref_->at(temp_cur) - grad_ref_->at(parents[temp_cur]));

            aux_price_(temp_cur) = temp;
          }
        }
      }
    }

    /* обратная трассировка */
    cur = parents[last];
    if (include_init_points) last_path_.push_back(last);
    double total_cost = grad_ref_->at(cur);
    while (cur != first) {
      total_cost += grad_ref_->at(cur);
      last_path_.push_back(cur);
      cur = parents[cur];
    }
    if (include_init_points) last_path_.push_back(first);

    if (last_path_.size() <= 3) return true;
    if (factor_ < Double::epsilon()) return true;
    if (total_cost >= last_path_.size()*medium_*factor_) return true;
    return false;
  }

  path_t PathFinder::lastPath() const {
    return last_path_;
  }

  void PathFinder::setGradientRef(Matrix<double>* gradient) {
    grad_ref_ = gradient;
    medium_ = grad_ref_->medium();
  }

  void PathFinder::setGradient(const Matrix<double>& gradient) {
    grad_ = gradient;
    setGradientRef(&grad_);
  }

  void PathFinder::setPassability(PassabilityMap::HardPtr map) {
    pass_map_ = map;
  }

  void PathFinder::scaleGradient(double down, double up) {
    grad_ref_->scale(down, up);
  }

  void PathFinder::setFactor(double factor) {
    factor_ = factor;
  }
}
