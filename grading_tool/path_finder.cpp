#include "path_finder.h"
#include <list>
#include <map>

#include "utils.h"

const int dx[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
const int dy[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };

std::pair<int, int> point2pair(const cv::Point& pt) {
  return std::make_pair(pt.x, pt.y);
}

cv::Point pair2point(const std::pair<int, int>& pt) {
  return cv::Point(pt.first, pt.second);
}

/* PathFinder */
PathFinder::PathFinder() {

}

PathFinder::PathFinder(const cv::Size& size) :
  aux_price_(size, 0),
  label_map_(size, 0),
  price_(size, 0) 
{

}

bool PathFinder::find(const cv::Point& first, const cv::Point& last, int flag, bool include_init_points) {
  last_path_.clear();
  if (first == last) return true;

  std::list<cv::Point> open(1, first);
  std::map<PointT, PointT> parents;
  label_map_(first) = ++label_;

  auto dist = [](const cv::Point& first, const cv::Point& second) {
    return std::sqrt((first.x - second.x) * (first.x - second.x) + (first.y - second.y) * (first.y - second.y));
  };

  double min_val, temp;
  cv::Point cur, temp_cur;
  std::list<cv::Point>::iterator min_ind;
  for (;;) {
    if (open.empty()) return false;

    min_val = DBL_MAX;
    for (auto it = open.begin(); it != open.end(); ++it) {
      if (min_val > aux_price_(*it)) {
        min_val = aux_price_(*it);
        min_ind = it;
      }
    }

    cur = *min_ind;
    if (cur == last) break;
    open.erase(min_ind);

    for (int i = 0; i < 8; ++i) {
      temp_cur = cv::Point(cur.x + dx[i], cur.y + dy[i]);
      if (temp_cur.x >= 0 && temp_cur.y >= 0 && temp_cur.x < label_map_.cols && temp_cur.y < label_map_.rows) {
        if (label_map_(temp_cur) != label_) {
          price_(temp_cur) = price_(cur) + dist(temp_cur, cur);
          label_map_(temp_cur) = label_;
          parents.emplace(std::make_pair(temp_cur.x, temp_cur.y), std::make_pair(cur.x, cur.y));
          open.push_back(temp_cur);

          temp = price_(temp_cur);
          if (flag & Gradient) temp += -grad_.at<double>(temp_cur);
          if (flag & GradientDiff) temp += abs(grad_.at<double>(temp_cur) - grad_.at<double>(pair2point(parents[point2pair(temp_cur)])));
          if (flag & Distance) temp += dist(temp_cur, last);

          aux_price_(temp_cur) = temp;
        }
      }
    }
  }

  /* обратная трассировка */
  cur = pair2point(parents[point2pair(last)]);
  if (include_init_points) last_path_.push_back(last);
  double total_cost = grad_.at<double>(cur);
  while (cur != first) {
    total_cost += grad_.at<double>(cur);
    last_path_.push_back(cur);
    cur = pair2point(parents[point2pair(cur)]);
  }
  if (include_init_points) last_path_.push_back(first);

  if (last_path_.size() <= 3) return true;
  if (factor_ < DBL_EPSILON) return true;
  if (total_cost >= last_path_.size() * medium_ * factor_) return true;

  return false;
}

std::vector<cv::Point> PathFinder::lastPath() const {
  return last_path_;
}

void PathFinder::setGradient(const cv::Mat_<double>& gradient) {
  grad_ = gradient;

  // TODO:
  // medium_ = grad_ref_->medium();
}

void PathFinder::scaleGradient(double down, double up) {
  scale(grad_, down, up);
}

void PathFinder::setFactor(double factor) {
  factor_ = factor;
}
