#include "image_info.h"
#include <stack>
#include <set>

namespace xr {
  namespace info
  {
    // TODO развернуть циклы
    bool hasFrame(const Image& image, uint8_t frame_color) {
      int top = image.height() - 1;
      int right = image.width() - 1;
      for (int i = 0; i<image.width(); ++i) {
        if (image.byte(i, 0) != frame_color || image.byte(i, top) != frame_color) {
          return false;
        }
      }

      for (int j = 0; j<image.height(); ++j) {
        if (image.byte(0, j) != frame_color || image.byte(right, j) != frame_color) {
          return false;
        }
      }

      return true;
    }

    bool isEmpty(const Image& image, uint8_t back_color) {
      uint8_t* cur = image.data();
      int pixels = image.width()*image.height();
      for (int i = 0; i<pixels; ++i) {
        if (*cur++ != back_color) return false;
      }

      return true;
    }

    int neighborsNumber(const Image& image, int x, int y) {
      return countAdjacentColor(image, x, y, image.byte(x, y));
    }

    std::vector<point_t> neighborsPixel(const Image& image, int x, int y) {
      std::vector<point_t> neighbors;
      for (int j = 0; j < 8; ++j) {
        if (image.byte(x + math::cdx[j], y + math::cdy[j]) == 255) {
          neighbors.emplace_back(x + math::cdx[j], y + math::cdy[j]);
        }
      }

      return neighbors;
    }

    std::vector<int> neighborsPixelIndices(const Image& image, int x, int y) {
      std::vector<int> neighbors;
      for (int j = 0; j < 8; ++j) {
        if (image.byte(x + math::dx[j], y + math::dy[j]) == 255) {
          neighbors.emplace_back(j);
        }
      }

      return neighbors;
    }

    bool isAdjacentColor(const Image& image, int x, int y, uint8_t color) {
      int tx, ty;
      for (int j = 0; j < 8; ++j) {
        tx = x + math::dx[j];
        ty = y + math::dy[j];
        if (image.isCorrect(tx, ty) && image.byte(tx, ty) == color) {
          return true;
        }
      }

      return false;
    }

    int countAdjacentColor(const Image& image, int x, int y, uint8_t color) {
      int tx, ty;
      int count = 0;
      for (int j = 0; j < 8; ++j) {
        tx = x + math::dx[j];
        ty = y + math::dy[j];
        if (image.isCorrect(tx, ty) && image.byte(tx, ty) == color) ++count;
      }

      return count;
    }

    bool isAdjacentColor4w(const Image& image, int x, int y, uint8_t color) {
      return 
        (y - 1 >= 0 && image.byte(x, y - 1) == color) ||
        (x - 1 >= 0 && image.byte(x - 1, y) == color) ||
        (y + 1<image.height() && image.byte(x, y + 1) == color) ||
        (x + 1<image.width() && image.byte(x + 1, y) == color);
    }

    int numberOfAssociatedPixels(const Image& image, int x, int y, Connectivity way, int limit) {
      uint8_t color = image.byte(x, y);
      std::stack<point_t> data;
      std::set<point_t> points;
      point_t coord;
      int tx, ty;

      int count = 0;
      data.emplace(x, y);
      do {
        coord = data.top();
        data.pop();

        if (count++ >= limit) {
          return limit;
        }

        points.insert(coord);
        for (int i = 0; i < way; ++i) {
          tx = coord.x + math::dx[i];
          ty = coord.y + math::dy[i];
          if (!image.isCorrect(tx, ty)) continue;
          if (image.byte(tx, ty) == color && !points.count(point_t(tx, ty))) {
            data.emplace(tx, ty);
          }
        }
      } while (!data.empty());

      return count;
    }
  }
}
