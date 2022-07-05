#pragma once
#include <functional>
#include "size.h"
#include "point.h"

namespace xr
{
  template<typename T>
  class Matrix
  {
    T* data_ = nullptr;
    int width_ = 0, height_ = 0;

    void release() {
      if (data_) {
        delete[] data_;
        data_ = nullptr;
      }

      width_ = height_ = 0;
    }

  public:
    Matrix() = default;

    Matrix(const Matrix<T>& other) :
      data_(new T[other.width_*other.height_]),
      width_(other.width_),
      height_(other.height_)
    {
      memcpy(data_, other.data_, sizeof(T)*width_*height_);
    }

    Matrix(Matrix<T>&& other) :
      data_(other.data_),
      width_(other.width_),
      height_(other.height_) 
    {
      other.data_ = nullptr;
    }

    Matrix(const Size& size, const T& val = 0) {
      recreate(size.width, size.height, val);
    }

    Matrix(int width, int height, const T& val = 0) {
      recreate(width, height, val);
    }

    ~Matrix() {
      if (data_) {
        delete[] data_;
      }
    }

    Matrix<T>& operator = (const Matrix<T>& rhs) {
      if (this == &rhs) return *this;

      if (width_*height_ != rhs.width_*rhs.height_) {
        release();
        width_ = rhs.width_;
        height_ = rhs.height_;
        data_ = new T[width_*height_];
      }

      memcpy(data_, rhs.data_, sizeof(T)*width_*height_);
      return *this;
    }

    Matrix<T>& operator = (Matrix<T>&& rhs) {
      if (this == &rhs) return *this;

      release();
      width_ = rhs.width_;
      height_ = rhs.height_;
      std::swap(data_, rhs.data_);

      return *this;
    }
   
    static Matrix<T> unite(const Matrix<T>& lhs, const Matrix<T>& rhs, std::function<T(T, T)> unite_func) {
      Matrix<T> dst(lhs.size());
      for (int i = 0; i < lhs.width(); ++i) {
        for (int j = 0; j < lhs.height(); ++j) {
          dst(i, j) = unite_func(lhs(i, j), rhs(i, j));
        }
      }

      return dst;
    }

    template<typename S>
    Matrix<T>& from(const Matrix<S>& src) {
      if (data_) delete[] data_;

      data_ = new T[src.width() * src.height()];
      width_ = src.width();
      height_ = src.height();

      for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
          at(i, j) = static_cast<T>(src(i, j));
        }
      }

      return *this;
    }

    template<typename S>
    Matrix<S> to() const {
      Matrix<S> dst(size());
      for (int i = 0; i<width_; ++i) {
        for (int j = 0; j<height_; ++j) {
          dst(i, j) = static_cast<S>((*this)(i, j));
        }
      }

      return dst;
    }

    void recreate(int width, int height, const T& val) {
      recreate(width, height);
      clear(val);
    }

    void recreate(int width, int height) {
      if (width_ * height_ != width * height) {
        release();
        data_ = new T[width * height];
      }

      width_ = width;
      height_ = height;
    }

    void swap(Matrix<T>& matrix) {
      std::swap(data_, matrix.data_);
      std::swap(width_, matrix.width_);
      std::swap(height_, matrix.height_);
    }

    T* data() const {
      return data_;
    }

    bool isNull() const {
      return !data_;
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

    bool isCorrect(const point_t& point) const {
      return isCorrect(point.x, point.y);
    }

    bool isCorrect(int x, int y) const {
      return (x >= 0 && y >= 0 && x < width_ && y < height_);
    }

    T& operator () (const point_t& point) {
      return data_[point.x + point.y*width_];
    }

    const T operator () (const point_t& point) const {
      return data_[point.x + point.y*width_];
    }

    T& operator () (int i, int j) {
      return data_[i + j*width_];
    }

    const T operator () (int i, int j) const {
      return data_[i + j*width_];
    }

    T& at(const point_t& point) {
      return data_[point.x + point.y*width_];
    }

    const T at(const point_t& point) const {
      return data_[point.x + point.y*width_];
    }

    T& at(int i, int j) {
      return data_[i + j*width_];
    }

    const T at(int i, int j) const {
      return data_[i + j*width_];
    }

    T* line(int j) {
      return data_ + j*width_;
    }

    const T* line(int j) const {
      return data_ + j*width_;
    }

    T sum() const {
      T acc = 0;
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        acc += *cur++;
      }

      return acc;
    }

    T medium() const {
      double acc = 0;
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        acc += *cur++;
      }

      return T(acc / (width_*height_));
    }

    T maximum() const {
      T* cur = data_;
      T fmax = *cur++;
      for (int i = 1, n = width_*height_; i < n; ++i) {
        if (*cur > fmax) fmax = *cur;
        ++cur;
      }

      return fmax;
    }

    T minimum() const {
      T* cur = data_;
      T fmin = *cur++;
      for (int i = 1, n = width_*height_; i < n; ++i) {
        if (*cur < fmin) fmin = *cur;
        ++cur;
      }

      return fmin;
    }

    Matrix<T>& transform(std::function<T(T)> func) {
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        *cur = func(*cur);
        ++cur;
      }

      return *this;
    }

    Matrix<T>& scale(T down, T up) {
      T* cur = data_;
      T fmin = minimum();
      double temp = double(up - down) / (maximum() - fmin);
      for (int i = 0, n = width_*height_; i<n; ++i) {
        *cur = T(down + (*cur - fmin) * temp);
        ++cur;
      }

      return *this;
    }

    Matrix<T> scaled(T down, T up) const {
      Matrix<T> clone(*this);
      return clone.scale(down, up);
    }

    Matrix<T>& transpose() {
      T* dst = new T[height_*width_];
      for (int i = 0; i<width_; ++i) {
        for (int j = 0; j < height_; ++j) {
          dst[i + j*width_] = data_[j + i*width_];
        }
      }

      delete[] data_;
      data_ = dst;

      std::swap(height_, width_);
      return *this;
    }

    Matrix<T>& clear(const T& val) {
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        *cur++ = val;
      }

      return *this;
    }

    static Matrix<T> makeSobelKernel() {
      Matrix<T> kernel(3, 3);

      kernel(0, 0) = -1.0; kernel(0, 1) = 0.0; kernel(0, 2) = 1.0;
      kernel(1, 0) = -2.0; kernel(1, 1) = 0.0; kernel(1, 2) = 2.0;
      kernel(2, 0) = -1.0; kernel(2, 1) = 0.0; kernel(2, 2) = 1.0;

      return kernel;
    }

    static Matrix<T> makeLaplace4Kernel() {
      Matrix<T> kernel(3, 3);
      kernel(0, 0) = 0; kernel(0, 1) = -1;  kernel(0, 2) = 0;
      kernel(1, 0) = -1; kernel(1, 1) = 5; kernel(1, 2) = -1;
      kernel(2, 0) = 0; kernel(2, 1) = -1;  kernel(2, 2) = 0;

      return kernel;
    }

    static Matrix<T> makeLaplace8Kernel() {
      Matrix<T> kernel(3, 3);
      kernel(0, 0) = 1; kernel(0, 1) = 1;  kernel(0, 2) = 1;
      kernel(1, 0) = 1; kernel(1, 1) = -8; kernel(1, 2) = 1;
      kernel(2, 0) = 1; kernel(2, 1) = 1;  kernel(2, 2) = 1;

      return kernel;
    }

    static Matrix<T> makeLaplace12Kernel() {
      Matrix<T> kernel(3, 3);
      /*а это сразу сложение изображения с лаплассианом*/
      kernel(0, 0) = 1; kernel(0, 1) = 2;  kernel(0, 2) = 1;
      kernel(1, 0) = 2; kernel(1, 1) = -12; kernel(1, 2) = 2;
      kernel(2, 0) = 1; kernel(2, 1) = 2;  kernel(2, 2) = 1;

      return kernel;
    }

    static Matrix<T> makeAveragingKernel(int radius) {
      Matrix<T> kernel(radius * 2 + 1, radius * 2 + 1);
      kernel.clear(1.0 / ((radius * 2 + 1) * (radius * 2 + 1)));

      return kernel;
    }

    static Matrix<T> makeGaussianForCannyKernel() {
      Matrix<T> kernel(5, 5);

      kernel(0, 0) = 2.0 / 159; kernel(0, 1) = 4.0 / 159;  kernel(0, 2) = 5.0 / 159;  kernel(0, 3) = 4.0 / 159;  kernel(0, 4) = 2.0 / 159;
      kernel(1, 0) = 4.0 / 159; kernel(1, 1) = 9.0 / 159;  kernel(1, 2) = 12.0 / 159; kernel(1, 3) = 9.0 / 159;  kernel(1, 4) = 4.0 / 159;
      kernel(2, 0) = 5.0 / 159; kernel(2, 1) = 12.0 / 159; kernel(2, 2) = 15.0 / 159; kernel(2, 3) = 12.0 / 159; kernel(2, 4) = 5.0 / 159;
      kernel(3, 0) = 4.0 / 159; kernel(3, 1) = 9.0 / 159;  kernel(3, 2) = 12.0 / 159; kernel(3, 3) = 9.0 / 159;  kernel(3, 4) = 4.0 / 159;
      kernel(4, 0) = 2.0 / 159; kernel(4, 1) = 4.0 / 159;  kernel(4, 2) = 5.0 / 159;  kernel(4, 3) = 4.0 / 159;  kernel(4, 4) = 2.0 / 159;

      return kernel;
    }

    static Matrix<T> makeGaussianKernel(int radius, double sigma, bool normalize = false) {
      int length = radius * 2 + 1;
      double denom = 2.0*sigma*sigma, sum = 0;
      Matrix<T> kernel(length, length);
      for (int i = 0; i<length; ++i) {
        for (int j = 0; j<length; ++j) {
          kernel(i, j) = std::exp(-(math::sqr(i - radius) + math::sqr(j - radius)) / denom) / (math::Pi*denom);
          sum += kernel(i, j);
        }
      }

      if (normalize) {
        for (int i = 0; i<length; ++i) {
          for (int j = 0; j<length; ++j) {
            kernel(i, j) /= sum;
          }
        }
      }

      return kernel;
    }
  };

  using matd = Matrix<double>;
  using matb = Matrix<bool>;
  using mati = Matrix<int>;
}
