#include <map>
#include <stack>
#include <iostream>
#include "utility.h"
#include "path_finder.h"
#include "image_info.h"

namespace xr
{
  rect_t createBoundingRect(const contour_t& src) {
    if (src.empty()) return rect_t();

    point_t temp;
    rect_t rect(src[0].x, src[0].x, src[0].y, src[0].y);
    for (size_t i = 0; i < src.size(); ++i) {
      temp = src[i];

      if (rect.top < temp.y) rect.top = temp.y;
      if (rect.left > temp.x) rect.left = temp.x;
      if (rect.right < temp.x) rect.right = temp.x;
      if (rect.bottom > temp.y) rect.bottom = temp.y;
    }

    return rect;
  }

  Orientation orientation(const contour_t& src) {
    size_t ind_top = 0;
    int top_y = Int::max();
    int bottom_y = Int::min();
    int center_x = createBoundingRect(src).center().x;
    /* ищем нужные индексы точек */
    for (size_t i = 0; i < src.size(); ++i) {
      if (src[i].x == center_x) {
        if (src[i].y < top_y) {
          ind_top = i;
          top_y = src[i].y;
        }
        
        if (src[i].y > bottom_y) bottom_y = src[i].y;
      }
    }

    int sum = 0;
    for (size_t i = ind_top;; ++i) {
      int real_ind = math::normalize(i, src.size());
      sum += src[real_ind].x - center_x;

      if (src[real_ind].x == center_x && src[real_ind].y == bottom_y) {
        break;
      }
    }

    return (sum > 0) ? Orientation::Clockwise : Orientation::AntiClockwise;
  }

  contour_t getContourByRosenfeld(const Image& image) {
    if (!info::hasFrame(image, 0) || info::isEmpty(image)) {
      throw std::logic_error("неподходящее изображение! нет рамки с фоном или пустое!");
    }

    std::map<point_t, int> mask = {
      { point_t(-1, -1), 0 },
      { point_t(0, -1), 1 },
      { point_t(1, -1), 2 },
      { point_t(1, 0), 3 },
      { point_t(1, 1), 4 },
      { point_t(0, 1), 5 },
      { point_t(-1, 1), 6 },
      { point_t(-1, 0), 7 }
    };

    /* определяем первую точку */
    int x = 0, y = 0;
    while (image.byte(x, y) != 255) {
      if (++x == image.width()) {
        x = 0; ++y;
      }
    }

    /* определим вторую точку */
    points_t dst = { make_point(x, y) };
    for (int i = 3; i < 7 && image.byte(x, y) != 255; ++i) {
      x = dst.front().x + math::cdx[i];
      y = dst.front().y + math::cdy[i];
    }

    /* остальные точки */
    int temp_x, temp_y;
    int start_index, real_index;
    do {
      dst.push_back(make_point(x, y));
      temp_x = (dst.end() - 2)->x - dst.back().x;
      temp_y = (dst.end() - 2)->y - dst.back().y;
      start_index = mask[make_point(temp_x, temp_y)] + 1;
      for (int i = start_index; i<start_index + 8 && image.byte(x, y) != 255; ++i) {
        real_index = math::normalize(i, 8);
        x = dst.back().x + math::cdx[real_index];
        y = dst.back().y + math::cdy[real_index];
      }

    } while (x != dst.front().x || y != dst.front().y);

    return dst;
  }

  point_t findNormalContourPoint(const Image& image) {
    Image aux(image);
    int top = aux.height() - 1;
    int right = aux.width() - 1;
    const uint8_t temp_color = 128;
    if (aux.byte(0, 0) == 0) aux.floodFill(0, 0, temp_color, Connectivity::Four);
    if (aux.byte(right, 0) == 0) aux.floodFill(right, 0, temp_color, Connectivity::Four);
    if (aux.byte(0, top) == 0) aux.floodFill(0, top, temp_color, Connectivity::Four);
    if (aux.byte(right, top) == 0) aux.floodFill(right, top, temp_color, Connectivity::Four);

    for (int j = 1; j < aux.height() - 1; ++j) {
      for (int i = 1; i < aux.width() - 1; ++i) {
        if (aux.byte(i, j) == 255 && info::neighborsNumber(image, i, j) == 2 &&
          info::isAdjacentColor4w(aux, i, j, 0) && info::isAdjacentColor4w(aux, i, j, temp_color)) 
        {
          return point_t(i, j);
        }
      }
    }

    throw std::runtime_error("error in findNormalContourPoint(..)!");
    return point_t();
  }

  contour_t toFundamental(const Image& src) {
    Matrix<int> labels(src.size(), 0);
    auto point = findNormalContourPoint(src);
    if (point.x == 0 && point.y == 0) {
      return contour_t();
    }

    points_t neighbors = info::neighborsPixel(src, point.x, point.y);
    if (neighbors.front().dist(neighbors.back()) <= 1) {
      throw std::runtime_error("error in leadToFundamental(..)!");
    }

    labels(point) = 1;
    labels(neighbors.back()) = 2;
    labels(neighbors.front()) = 0xffff;
    std::stack<point_t> lstack, rstack, tmp_stack;
    lstack.push(neighbors.back());
    rstack.push(neighbors.front());
    /* пускаем две волны в разные стороны, пока не встретятся */
    int x, y;
    point_t cur;
    points_t lhs, rhs;
    while (rhs.empty()) {
      // левая волна
      tmp_stack = std::stack<point_t>();
      while (!lstack.empty()) {
        cur = lstack.top();
        lstack.pop();
        for (int i = 0; i < 8; ++i) {
          x = cur.x + math::cdx[i];
          y = cur.y + math::cdy[i];
          if (src.byte(x, y) == 255 && !labels(x, y)) {
            tmp_stack.push(point_t(x, y));
            labels(x, y) = labels(cur) + 1;
          }
        }
      }

      lstack.swap(tmp_stack);

      // правая волна
      while (!rstack.empty()) {
        cur = rstack.top();
        rstack.pop();
        for (int i = 0; i < 8; ++i) {
          x = cur.x + math::cdx[i];
          y = cur.y + math::cdy[i];
          if (src.byte(x, y) == 255) {
            if (!labels(x, y)) {
              tmp_stack.push(point_t(x, y));
              labels(x, y) = labels(cur) + 1;
            }
            else if (1 < labels(x, y) && labels(x, y) < 0xffff) {
              // волны встретились
              rhs.push_back(cur);
              lhs.push_back(point_t(x, y));
              break;
            }
          }
        }
      }

      rstack.swap(tmp_stack);
    }

    /* обратная трассировка */
    int counter = labels(lhs.back());
    while (labels(lhs.back()) != 1) { // lhs
      --counter;
      cur = lhs.back();
      for (int i = 0; i<8; ++i) {
        if (labels(point_t(cur.x + math::dx[i], cur.y + math::dy[i])) == counter) {
          lhs.push_back(point_t(cur.x + math::dx[i], cur.y + math::dy[i]));
          break;
        }
      }
    }

    counter = labels(rhs.back());
    while (labels(rhs.back()) != 0xffff) { // rhs
      --counter;
      cur = rhs.back();
      for (int i = 0; i<8; ++i) {
        if (labels(point_t(cur.x + math::dx[i], cur.y + math::dy[i])) == counter) {
          rhs.push_back(point_t(cur.x + math::dx[i], cur.y + math::dy[i]));
          break;
        }
      }
    }

    rhs.insert(rhs.begin(), lhs.rbegin(), lhs.rend());

    /* вспомним про рамку, учтем в координатах */
    for (auto& it : rhs) {
      it -= point_t(1, 1);
    }

    /* ВАЖНО! обход контура должен производиться по часовой стрелке; поэтому такой return */
    if (orientation(rhs) == Orientation::Clockwise) return rhs;
    return contour_t(rhs.rbegin(), rhs.rend());
  }

  contour_t toFundamental(const contour_t& src) {
    if (src.size() < 16) return contour_t();

    Image pixmap(std::move(Image::draw(src, 1)));
    contour_t result;
    try {
      result = toFundamental(pixmap);

      if (!result.empty()) { // восстановим координаты
        auto rect = createBoundingRect(src);
        for (auto it = result.begin(); it != result.end(); ++it) {
          it->x += rect.left;
          it->y += rect.bottom;
        }
      }
    }
    catch (const std::exception& e) {
      xr::imwrite(pixmap, "broken-image.png");
      std::cerr << "exception: " << e.what() << std::endl;
      result = src;
    }

    return result;
  }

  contour_t& simplify(contour_t& src, int k) {
    size_t n = src.size(), j = 0;
    for (size_t i = 0; i < n; i += k, j += 1) {
      src[j] = src[i];
    }

    src.resize(j);
    return src;
  }

  contour_t& amplify(contour_t& src, PathFinder* finder, int flags) {
    int next;
    contour_t result;
    for (size_t i = 0; i < src.size(); ++i) {
      next = (i + 1) % src.size();
      if (src[i] == src[next]) continue;

      finder->find(src[i], src[next], flags, true); // игнорим результат

      auto temp = finder->lastPath();
      result.insert(result.end(), temp.rbegin(), temp.rend());
    }
    
    src.swap(result);
    return src;
  }

  points_t makeDirectPath(point_t start, point_t finish) {
    points_t path;
    int x1 = start.x, x2 = finish.x, y1 = start.y, y2 = finish.y;
    if (abs(x1 - x2) > abs(y1 - y2)) {
      int step= math::sign(x2 - x1);
      for (int x = start.x; x != finish.x; x += step) {
        double y = (x - x1) * (y2 - y1) * 1.0 / (x2 - x1) + y1;
        path.emplace_back(x, static_cast<int>(y));
      }
    }
    else {
      int step = math::sign(y2 - y1);
      for (int y = start.y; y != finish.y; y += step) {
        double x = (y - y1) * (x2 - x1) * 1.0 / (y2 - y1) + x1;
        path.emplace_back(static_cast<int>(x), y);
      }
    }

    path.push_back(finish);
    return path;
  }

  double rmsError(const contour_t& first, const contour_t& other) {
    contour_t second(first.size());
    for (size_t i = 0; i < first.size(); ++i) {
      point_t target;
      auto& current = first[i];
      double min_dist = Double::max();
      for (auto& e : other) {
        auto d = xr::dist(current, e);
        if (d < min_dist) {
          min_dist = d;
          target = e;
        }
      }

      second[i] = target;
    }

    double sum = 0.0;
    for (size_t i = 0; i < first.size(); ++i) {
      sum += xr::sqrDist(first[i], second[i]);
    }

    return sqrt(sum / first.size());
  }

  double meanDistance(const contour_t& first, const contour_t& other) {
    contour_t second(first.size());
    for (size_t i = 0; i < first.size(); ++i) {
      point_t target;
      auto& current = first[i];
      double min_dist = Double::max();
      for (auto& e : other) {
        auto d = xr::dist(current, e);
        if (d < min_dist) {
          min_dist = d;
          target = e;
        }
      }

      second[i] = target;
    }

    double sum = 0.0;
    for (size_t i = 0; i < first.size(); ++i) {
      sum += xr::dist(first[i], second[i]);
    }

    return sum / first.size();
  }

  double standartDeviation(const contour_t& first, const contour_t& other) {
    contour_t second(first.size());
    for (size_t i = 0; i < first.size(); ++i) {
      point_t target;
      auto& current = first[i];
      double min_dist = Double::max();
      for (auto& e : other) {
        auto d = xr::dist(current, e);
        if (d < min_dist) {
          min_dist = d;
          target = e;
        }
      }

      second[i] = target;
    }

    double mean = 0.0;
    for (size_t i = 0; i < first.size(); ++i) {
      mean += xr::dist(first[i], second[i]);
    }

    mean /= first.size();

    double dev = 0.0;
    for (size_t i = 0; i < first.size(); ++i) {
      dev += xr::math::sqr(xr::dist(first[i], second[i]) - mean);
    }

    return sqrt(dev / first.size());
  }

  double hausdorfDistance(const contour_t& first, const contour_t& other) {
    contour_t second(first.size());
    for (size_t i = 0; i < first.size(); ++i) {
      point_t target;
      auto& current = first[i];
      double min_dist = Double::max();
      for (auto& e : other) {
        auto d = xr::dist(current, e);
        if (d < min_dist) {
          min_dist = d;
          target = e;
        }
      }

      second[i] = target;
    }

    size_t n = first.size();
    double sup_x = Double::min(), sup_y = Double::min();
    for (size_t i = 0; i < n; ++i) {
      double inf_x = Double::max(), inf_y = Double::max();
      for (size_t k = 0; k < n; ++k) {
        inf_x = std::min(inf_x, xr::dist(first[i], second[k]));
        inf_y = std::min(inf_y, xr::dist(second[i], first[k]));
      }

      sup_x = std::max(sup_x, inf_x);
      sup_y = std::max(sup_y, inf_y);
    }

    return std::max(sup_x, sup_y);
  }
}