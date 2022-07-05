#include "rgb_image.h"
#include <assert.h>
#include "except.h"

#include <opencv2/opencv.hpp>

namespace xr
{
  RgbImage::RgbImage(const RgbImage& src) {
    create(src.data_, src.width_, src.height_);
  }

  RgbImage::RgbImage(RgbImage&& src) :
    data_(src.data_),
    height_(src.height_),
    width_(src.width_)
  {
    src.data_ = nullptr;
  }
  
  RgbImage::RgbImage(const Image& src):
    data_(new uint8_t[src.width()*src.height() * 3]),
    width_(src.width()),
    height_(src.height()) 
  {
    int pixels = width_*height_;

    uint8_t* cur = data_;
    uint8_t* cur_src = src.data();
    for (int i = 0; i < pixels; ++i) {
      *cur++ = *cur_src;
      *cur++ = *cur_src;
      *cur++ = *cur_src++;
    }
  }

  RgbImage::RgbImage(const Size& size) :
    data_(new uint8_t[size.width*size.height * 3]),
    height_(size.height),
    width_(size.width) 
  {

  }

  RgbImage::RgbImage(int width, int height) :
    data_(new uint8_t[width*height * 3]),
    height_(height),
    width_(width) 
  {

  }

  RgbImage::RgbImage(const Size& size, const Color& color) :
    data_(new uint8_t[size.width*size.height * 3]),
    height_(size.height),
    width_(size.width) 
  {
    clear(color);
  }

  RgbImage::RgbImage(int width, int height, const Color& color):
    data_(new uint8_t[width*height * 3]),
    height_(height),
    width_(width)
  {
    clear(color);
  }

  RgbImage::RgbImage(uint8_t* src, const Size& size) {
    create(src, size.width, size.height);
  }

  RgbImage::RgbImage(uint8_t* src, int width, int height) {
    create(src, width, height);
  }

  RgbImage::~RgbImage() {
    if (data_) {
      delete[] data_;
    }
  }

  RgbImage& RgbImage::swap(RgbImage& other) {
    std::swap(data_, other.data_);
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    return *this;
  }

  bool RgbImage::load(const char* filename) {
    auto sample = cv::imread(filename, cv::IMREAD_COLOR);
    if (!sample.empty()) {
      width_ = sample.cols;
      height_ = sample.rows;
      int bytes = width_ * height_ * 3;
      data_ = new uint8_t[bytes];

      memcpy(data_, sample.data, bytes * sizeof(uint8_t));

      return true;
    }

    return false;
  }

  bool RgbImage::save(const char* filename) {
    cv::Mat image(cv::Size(width_, height_), CV_8UC3, data_);
    return cv::imwrite(filename, image);
  }

  void RgbImage::reset() {
    if (data_) delete[] data_;

    width_ = 0;
    height_ = 0;
    data_ = nullptr;
  }
  
  void RgbImage::create(int width, int height) {
    int bytes = width*height * 3;
    if (!data_ || width_*height_ != width*height) {
      reset();
      data_ = new uint8_t[bytes];
    }

    width_ = width;
    height_ = height;
  }

  void RgbImage::create(uint8_t* src, int width, int height) {
    create(width, height);
    int bytes = width*height * 3;
    memcpy(data_, src, bytes*sizeof(uint8_t));
  }

  bool RgbImage::operator == (const RgbImage& other) {
    uint8_t* cur = data_;
    uint8_t* other_cur = other.data();
    int bytes = width_ * height_ * 3;
    for (int i = 0; i < bytes; ++i) {
      if (*cur++ != *other_cur++) return false;
    }

    return true;
  }

  RgbImage& RgbImage::operator = (RgbImage&& src) {
    if (this == &src) return *this;

    if (data_) reset();

    data_ = src.data_;
    width_ = src.width_;
    height_ = src.height_;
    src.data_ = nullptr;

    return *this;
  }

  RgbImage& RgbImage::operator = (const RgbImage& src) {
    if (this == &src) return *this;

    create(src.data_, src.width_, src.height_);

    return *this;
  }

  RgbImage& RgbImage::operator = (const Image& src) {
    int pixels = width_*height_;
    create(src.width(), src.height());

    uint8_t* cur = data_;
    uint8_t* cur_src = src.data();
    for (int i = 0; i < pixels; ++i) {
      *cur++ = *cur_src;
      *cur++ = *cur_src;
      *cur++ = *cur_src++;
    }

    return *this;
  }

  RgbImage& RgbImage::clear(const Color& color) {
    int pixels = width_ * height_;
    uint8_t* cur = data_;
    for (int i = 0; i < pixels; ++i) {
      *cur++ = color.r;
      *cur++ = color.g;
      *cur++ = color.b;
    }

    return *this;
  }

  RgbImage& RgbImage::hFlip() {
    Color temp;
    for (int j = 0; j<height_; ++j) {
      for (int i = 0, w = width_ / 2; i <= w; ++i) {
        temp = getColor(i, j);
        setColor(i, j, getColor(width_ - 1 - i, j));
        setColor(width_ - 1 - i, j, temp);
      }
    }

    return *this;
  }

  RgbImage& RgbImage::vFlip() {
    Color temp;
    for (int i = 0; i<width_; ++i) {
      for (int j = 0, h = height_ / 2; j <= h; ++j) {
        temp = getColor(i, j);
        setColor(i, j, getColor(i, height_ - 1 - j));
        setColor(i, height_ - 1 - j, temp);
      }
    }

    return *this;
  }

  RgbImage& RgbImage::turnClockwise() {
    RgbImage dst(height_, width_);
    for (int i = 0; i<width_; ++i) {
      for (int j = 0; j<height_; ++j) {
        dst.setColor(j, i, getColor(i, j));
      }
    }

    swap(dst);
    return *this;
  }

  RgbImage& RgbImage::turnCounterClockwise() {
    return turnClockwise().hFlip();
  }

  RgbImage& RgbImage::invert() {
    uint8_t* cur = data_;
    int bytes = width_*height_ * 3;
    for (int i = 0; i < bytes; ++i) {
      *cur = 255 - (*cur);
      ++cur;
    }

    return *this;
  }

  RgbImage& RgbImage::grayscale() {
    uint8_t pixel;
    uint8_t* cur = data_;
    int pixels = width_*height_;
    for (int i = 0; i < pixels; ++i) {
      pixel = static_cast<uint8_t>((*cur)*0.114 + (*(cur + 1))*0.587 + (*(cur + 2))*0.299);
      *cur++ = pixel;
      *cur++ = pixel;
      *cur++ = pixel;
    }

    return *this;
  }

  RgbImage RgbImage::subimage(int x, int y, int width, int height) {
    RgbImage dst(width, height);
    for (int i = 0; i<width; ++i) {
      for (int j = 0; j<height; ++j) {
        dst.setColor(i, j, getColor(i + x, j + y));
      }
    }

    return dst;
  }

  Image RgbImage::toGray() const {
    Image dst(size());
    int pixels = width_ * height_;
    uint8_t* cur_dst = dst.data();
    Color* cur_src = (Color*)data_;
    for (int i = 0; i < pixels; ++i) {
      *cur_dst++ = (*cur_src).toGray();
      ++cur_src;
    }

    return dst;
  }
}