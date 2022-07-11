#pragma once
#include "defs.h"
#include "except.h"
#include "matrix.h"
#include "rect.h"

//#define NO_THROW_EXCEPTIONS

#undef max

namespace xr
{
  // TODO в каждом изобржаении хранить буфер для вычислений (или создавать его по флагу)
  class Image {
    uint8_t* data_ = nullptr;
    int width_ = 0, height_ = 0;

    void recreate(int width, int height);
    void release();

  public:
    Image() = default;

    Image(const Image& src);
    Image(Image&& src);

    Image(const cv::Size& size);
    Image(int width, int height);

    ~Image();

    template<typename T>
    Image(const Matrix<T>& src) :
      data_(new uint8_t[src.width()*src.height()]),
      width_(src.width()),
      height_(src.height()) 
    {
      for (int i = 0; i<width_; ++i) {
        for (int j = 0; j<height_; ++j) {
          byte(i, j) = static_cast<uint8_t>(src(i, j));
        }
      }
    }

    Image& operator = (const Image& rhs);
    Image& operator = (Image&& rhs);

    Image& swap(Image& other);

    Image clone() const;

    template<typename T>
    std::vector<T> histogram() const {
      uint8_t* cur = data_;
      std::vector<T> dst(256, 0);
      int pixels = width_*height_;
      for (int i = 0; i < pixels; ++i) {
        dst[*cur++] += 1;
      }

      return dst;
    }

    template<typename T>
    Image& from(const Matrix<T>& src) {
      recreate(src.width(), src.height());
      for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
          byte(i, j) = static_cast<uint8_t>(src(i, j));
        }
      }

      return *this;
    }

    template<typename T>
    Matrix<T> to() const {
      Matrix<T> dst(size());

      for (int i = 0; i<width_; ++i) {
        for (int j = 0; j<height_; ++j) {
          dst(i, j) = static_cast<T>(byte(i, j));
        }
      }

      return dst;
    }

    bool isNull() const {
      return !data_ || !width_ || !height_;
    }

    uint8_t* data() const {
      return data_;
    }

    cv::Size size() const {
      return cv::Size(width_, height_);
    }

    int height() const {
      return height_;
    }

    int width() const {
      return width_;
    }

    bool isCorrect(int x, int y) const {
      return x >= 0 && y >= 0 && x<width_ && y<height_;
    }

    bool isCorrect(const point_t& point) const {
      return isCorrect(point.x, point.y);
    }

    uint8_t* row(int line) {
      return (data_ + (height_ - line - 1)*width_);
    }

    uint8_t& byte(int x, int y) {
#ifndef NO_THROW_EXCEPTIONS
      if (!isCorrect(x, y)) throw OutOfRangeException();
#endif
      return data_[x + (height_ - y - 1)*width_];
    }

    uint8_t& byte(const point_t& point) {
      return byte(point.x, point.y);
    }

    const uint8_t byte(int x, int y) const {
#ifndef NO_THROW_EXCEPTIONS
      if (!isCorrect(x, y)) throw OutOfRangeException();
#endif
      return data_[x + (height_ - y - 1)*width_];
    }

    const uint8_t byte(const point_t& point) const {
      return byte(point.x, point.y);
    };
    
    static Image draw(const contour_t& src, int margin = 0, uint8_t color = 255);

    int sum() const;

    Image& binarization(uint8_t threshold);

    uint8_t thresholdByOtsu() const;
    uint8_t thresholdByBasedGradient() const;

    Matrix<double> convolution(const Matrix<double>& kernel) const;

    // основные функции для вычисления градиента ихображения
    void gradient(Matrix<double>& u, Matrix<double>& v) const;
    void gradient(const Matrix<double>& kernel, Matrix<double>& u, Matrix<double>& v) const;

    // вспомогательные ф-и, для одиночных операций (где не требуются многоразового использования обеих компонент)
    Matrix<double> gradient(std::function<double(double, double)> value_in_point) const;
    Matrix<double> gradient(const Matrix<double>& kernel, std::function<double(double, double)> value_in_point) const;

    Image& erode(int radius);
    Image& dilate(int radius);
    Image& closing(int radius);
    Image& opening(int radius);

    Image& nonMaximumSuppression(const matd& directon);

    Image& bilateralFiltering(double sigmaS, double sigmaR); 
    Image& gaussianBlur(int radius, double sigma); // TODO соотнощение между радиусом и сигмой взять из OpenCV
    Image& gaussianBlurForCanny();
    Image& medianBlur(int radius);
    Image& kuwahara(int radius);
    Image& laplace(int mode = 4);
    Image& kirsch();
    Image& sobel();

    Image& clear(const uint8_t& val);
    Image& histogramEqualize();
    Image& contrast();
    Image& invert();

    Image& hFlip();
    Image& vFlip();
    Image& trim(int margin);
    Image& trim(uint8_t color = 0);
    Image& setFrame(int border_width, uint8_t color);
    Image& changeColor(uint8_t old_color, uint8_t new_color);
    Image& changeColor(bool(*pred)(uint8_t), uint8_t new_color);
    Image& turnClockwise();
    Image& turnCounterClockwise();

    Image subimage(const rect_t& roi);
    Image subimage(int x, int y, int width, int height);

    Image& floodFill(int x, int y, uint8_t color, xr::Connectivity way = xr::Four);

    Image& fillSmallAreas(size_t max_region_size);
    points_t getPointsRegion(int x, int y, xr::Connectivity way = xr::Four, int upper_limit = Int::max()) const;

    void gvf(double mu, int iters, Matrix<double>& u, Matrix<double>& v);

    Image gvf(double mu, int iters, std::function<double(double, double)> unite_func);
  };

  Image imread(const std::string& filename);
  bool imwrite(const Image& image, const std::string& filename);

  Image* draw(Image* image, const contour_t& contour, uint8_t color);
  Image* draw(Image* image, const contours_t& contours, uint8_t color);
}
