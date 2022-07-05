#pragma once
#include <memory>
#include "defs.h"
#include "size.h"
#include "point.h"
#include "color.h"
#include "image.h"

#undef NO_THROW_EXCEPTIONS

namespace xr
{
  class RgbImage {
    uint8_t* data_ = nullptr;
    int width_ = 0, height_ = 0;

    void reset();
    void create(int width, int height);
    void create(uint8_t* src, int width, int height);

  public:
    RgbImage() = default;

    RgbImage(const RgbImage& src);
    RgbImage(RgbImage&& src);

    RgbImage(const Image& src);

    RgbImage(const Size& size);
    RgbImage(int width, int height);
    RgbImage(const Size& size, const Color& color);
    RgbImage(int width, int height, const Color& color);
    RgbImage(uint8_t* src, const Size& size);
    RgbImage(uint8_t* src, int width, int height);

    ~RgbImage();

    RgbImage& swap(RgbImage& other);

    bool load(const char* filename);
    bool save(const char* filename);

    bool operator == (const RgbImage& src);
    RgbImage& operator = (RgbImage&& src);
    RgbImage& operator = (const RgbImage& src);
    RgbImage& operator = (const Image& src);

    bool isNull() const {
      return !data_ || !width_ || !height_;
    }

    uint8_t* data() const {
      return data_;
    }

    Size size() const {
      return Size(width_, height_);
    }
    int height() const {
      return height_;
    }
    int width() const {
      return width_;
    }

    void setColor(int x, int y, const Color& color) {
      uint8_t* cur = data_ + (x + (height_ - y - 1)*width_) * 3;
      *cur++ = color.r;
      *cur++ = color.g;
      *cur = color.b;
    }
    void setColor(const point_t& point, const Color& color) {
      setColor(point.x, point.y, color);
    }

    Color getColor(int x, int y) const {
      uint8_t* cur = data_ + (x + (height_ - y - 1)*width_) * 3;
      return Color(*cur, *(cur + 1), *(cur + 2));
    }
    Color getColor(const point_t& point) const {
      return getColor(point.x, point.y);
    }

    RgbImage& clear(const Color& color);

    RgbImage& hFlip();
    RgbImage& vFlip();
    RgbImage& turnClockwise();
    RgbImage& turnCounterClockwise();

    RgbImage& invert();
    RgbImage& grayscale();

    RgbImage subimage(int x, int y, int width, int height);

    Image toGray() const;
  };
}
