#include "image.h"
#include <stack>
#include <utility>
#include <iostream>
#include <algorithm>
#include "utility.h"
#include "xr_math.h"

#include <opencv2/opencv.hpp>

namespace xr
{
  /* Image */
  Image::Image(const Image& src):
    data_(new uint8_t[src.width_*src.height_]),
    width_(src.width_),
    height_(src.height_) {
    memcpy(data_, src.data_, sizeof(uint8_t)*width_*height_);
  }

  Image::Image(Image&& src):
    data_(src.data_),
    width_(src.width_),
    height_(src.height_) {
    src.data_ = nullptr;
  }

  Image::Image(const cv::Size& size) {
    recreate(size.width, size.height);
  }

  Image::Image(int width, int height) {
    recreate(width, height);
  }

  Image::~Image() {
    if (data_) {
      delete[] data_;
    }
  }

  Image& Image::operator = (const Image& rhs) {
    if (this == &rhs) return *this;

    if (width_*height_ != rhs.width_*rhs.height_) {
      release();
      width_ = rhs.width_;
      height_ = rhs.height_;
      data_ = new uint8_t[width_*height_];
    }

    memcpy(data_, rhs.data_, sizeof(uint8_t)*width_*height_);
    return *this;
  }

  Image& Image::operator = (Image&& rhs) {
    if (this == &rhs) return *this;

    release();
    width_ = rhs.width_;
    height_ = rhs.height_;
    std::swap(data_, rhs.data_);

    return *this;
  }

  Image& Image::swap(Image& other) {
    std::swap(data_, other.data_);
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    return *this;
  }

  Image Image::clone() const {
    return Image(*this);
  }

  void Image::recreate(int width, int height) {
    if (width_ * height_ != width * height) {
      release();
      data_ = new uint8_t[width * height];
    }

    width_ = width;
    height_ = height;
  }

  void Image::release() {
    if (data_) {
      delete[] data_;
      data_ = nullptr;
    }

    width_ = height_ = 0;
  }

  int Image::sum() const {
    int sum = 0;
    uint8_t* cur = data_;
    for (int i = 0, n = width_*height_; i < n; ++i) {
      sum += *cur++;
    }

    return sum;
  }

  Image Image::draw(const contour_t& src, int margin, uint8_t color) {
    auto rect = createBoundingRect(src);
    Image target(rect.right - rect.left + 1 + margin * 2, rect.top - rect.bottom + 1 + margin * 2);
    target.clear(0);

    size_t n = src.size();
    int dx = -rect.left + margin;
    int dy = -rect.bottom + margin;
    for (size_t i = 0; i < n; ++i) {
      target.byte(src[i].x + dx, src[i].y + dy) = color;
    }

    return target;
  }

  Image& Image::binarization(uint8_t threshold) {
    uint8_t* cur = data_;
    int pixels = width_ * height_;
    for (int i = 0; i < pixels; ++i) {
      *cur = ((*cur) <= threshold) ? 0 : 255;
      cur++;
    }

    return *this;
  }

  uint8_t Image::thresholdByOtsu() const {
    int pixels = width_*height_;
    std::vector<double> p = histogram<double>();
    for (int i = 0; i < 256; ++i) p[i] /= pixels;

    double w1 = 0, n1 = 0, n2 = 0;
    for (size_t i = 0, n = p.size(); i < n; ++i) {
      n2 += i*p[i];
    }

    int threshold = 0;
    double t_val = 0, sigma, temp;

    for (int i = 0; i < 256; ++i) {
      w1 += p[i];
      n1 += i*p[i];
      n2 -= i*p[i];
      temp = n1 / w1 - n2 / (1.0 - w1);
      sigma = w1*(1.0 - w1)*temp*temp;

      if (t_val < sigma) {
        t_val = sigma;
        threshold = i;
      }
    }

    return static_cast<uint8_t>(threshold);
  }

  uint8_t Image::thresholdByBasedGradient() const {
    double Gx, Gy;
    double num = 0, denom = 1, temp;
    for (int i = 1; i < width_ - 1; ++i) {
      for (int j = 1; j < height_ - 1; ++j) {
        Gx = abs(byte(i + 1, j) - byte(i - 1, j));
        Gy = abs(byte(i, j + 1) - byte(i, j - 1));
        temp = xr::math::max(Gx, Gy);
        num += byte(i, j)*temp;
        denom += temp;
      }
    }

    return static_cast<uint8_t>(round(num / denom));
  }

  Matrix<double> Image::convolution(const Matrix<double>& kernel) const {
    Matrix<double> dst(size(), 0);
    int r = kernel.width() / 2;

    const int step = 4;

    /* центральная область */
    double t;
    int i, j, dx, dy;
    for (j = r; j < height_ - r - step; j += step) {
      for (i = r; i < width_ - r; ++i) {
        for (dy = -r; dy <= r; dy++) {
          for (dx = -r; dx <= r; dx++) {
            t = kernel(dx + r, dy + r);
            dst(i, j) += byte(i + dx, j + dy) * t;
            dst(i, j + 1) += byte(i + dx, j + 1 + dy) * t;
            dst(i, j + 2) += byte(i + dx, j + 2 + dy) * t;
            dst(i, j + 3) += byte(i + dx, j + 3 + dy) * t;
          }
        }
      }
    }

    /* снизу (+ остаток) */
    int jj;
    int tempH = 2 * height_ - 1;
    int temp = 2 * width_ - 1, temp_ind;
    while (j < height_) {
      for (i = 0; i < width_; ++i) {
        for (dy = -r; dy <= r; dy++) {
          jj = (j + dy >= height_) ? tempH - j - dy : j + dy;
          for (dx = -r; dx <= r; dx++) {
            temp_ind = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
            dst(i, j) += byte(temp_ind, jj) * kernel(dx + r, dy + r);
          }
        }
      }

      ++j;
    }

    /* слева */
    for (j = r; j < height_ - r; ++j) {
      for (i = 0; i < r; ++i) {
        for (dy = -r; dy <= r; dy++)
          for (dx = -r; dx <= r; dx++) {
            dst(i, j) += byte(abs(i + dx), j + dy)*kernel(dx + r, dy + r);
          }
      }
    }

    /* справа */
    for (j = r; j < height_ - r; ++j) {
      for (i = width_ - r; i < width_; ++i) {
        for (dy = -r; dy <= r; dy++) {
          for (dx = -r; dx <= r; dx++) {
            temp_ind = (i + dx >= width_) ? temp - i - dx : i + dx;
            dst(i, j) += byte(temp_ind, j + dy)*kernel(dx + r, dy + r);
          }
        }
      }
    }

    /* сверху */
    for (j = 0; j < r; ++j) {
      for (i = 0; i < width_; ++i) {
        for (dy = -r; dy <= r; dy++) {
          for (dx = -r; dx <= r; dx++) {
            temp_ind = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
            dst(i, j) += byte(temp_ind, abs(j + dy))*kernel(dx + r, dy + r);
          }
        }
      }
    }

    return dst;
  }

  void Image::gradient(Matrix<double>& u, Matrix<double>& v) const {
    u.recreate(width(), height(), 0.0);
    v.recreate(width(), height(), 0.0);

    for (int j = 1; j < height_ - 1; ++j) {
      for (int i = 1; i < width_ - 1; ++i) {
        u(i, j) = 0.5*(byte(i + 1, j) - byte(i - 1, j));
        v(i, j) = 0.5*(byte(i, j + 1) - byte(i, j - 1));
      }
    }

    /* на краях определена только либо fx, либо fy - другая равна нулю */
    for (int j = 0; j < height_; ++j) {
      u(0, j) = 0.5*(byte(1, j) - byte(0, j));
      u(width_ - 1, j) = 0.5*(byte(width_ - 1, j) - byte(width_ - 2, j));
    }

    for (int i = 0; i < width_; ++i) {
      v(i, 0) = 0.5*(byte(i, 1) - byte(i, 0));
      v(i, height_ - 1) = 0.5*(byte(i, height_ - 1) - byte(i, height_ - 2));
    }
  }

  void Image::gradient(const Matrix<double>& kernel, Matrix<double>& u, Matrix<double>& v) const {
    u.recreate(width(), height(), 0.0);
    v.recreate(width(), height(), 0.0);

    Matrix<double> dst(size(), 0.0);
    int r = kernel.width() / 2;

    // TODO

    /* центральная область */
    double fx, fy;
    for (int j = r; j < height_ - r; ++j) {
      for (int i = r; i < width_ - r; ++i) {
        fx = fy = 0;
        for (int dx = -r; dx <= r; dx++) {
          for (int dy = -r; dy <= r; dy++) {
            fx += byte(i + dx, j + dy)*kernel(dx + r, dy + r);
            fy += byte(i + dx, j + dy)*kernel(dy + r, dx + r);
          }
        }

        u(i, j) = fx;
        v(i, j) = fy;
      }
    }

    /* слева */
    for (int j = r; j < height_ - r; ++j) {
      for (int i = 0; i < r; ++i) {
        fx = fy = 0;
        for (int dx = -r; dx <= r; dx++) {
          for (int dy = -r; dy <= r; dy++) {
            fx += byte(abs(i + dx), j + dy)*kernel(dx + r, dy + r);
            fy += byte(abs(i + dx), j + dy)*kernel(dy + r, dx + r);
          }
        }

        u(i, j) = fx;
        v(i, j) = fy;
      }
    }

    /* справа */
    int temp = 2 * width_ - 1, temp_ind;
    for (int j = r; j < height_ - r; ++j) {
      for (int i = width_ - r - 1; i < width_; ++i) {
        fx = fy = 0;
        for (int dx = -r; dx <= r; dx++) {
          for (int dy = -r; dy <= r; dy++) {
            temp_ind = (i + dx >= width_) ? temp - i - dx : i + dx;
            fx += byte(temp_ind, j + dy)*kernel(dx + r, dy + r);
            fy += byte(temp_ind, j + dy)*kernel(dy + r, dx + r);
          }
        }

        u(i, j) = fx;
        v(i, j) = fy;
      }
    }

    /* сверху */
    for (int j = 0; j < r; ++j) {
      for (int i = 0; i < width_; ++i) {
        fx = fy = 0;
        for (int dx = -r; dx <= r; dx++) {
          for (int dy = -r; dy <= r; dy++) {
            temp_ind = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
            fx += byte(temp_ind, abs(j + dy))*kernel(dx + r, dy + r);
            fy += byte(temp_ind, abs(j + dy))*kernel(dy + r, dx + r);
          }
        }

        u(i, j) = fx;
        v(i, j) = fy;
      }
    }

    /* снизу */
    int tempH = 2 * height_ - 1;
    for (int j = height_ - r; j < height_; ++j) {
      for (int i = 0; i < width_; ++i) {
        fx = fy = 0;
        for (int dx = -r; dx <= r; dx++) {
          for (int dy = -r; dy <= r; dy++) {
            temp_ind = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
            fx += byte(temp_ind, (j + dy >= height_) ? tempH - j - dy : j + dy)*kernel(dx + r, dy + r);
            fy += byte(temp_ind, (j + dy >= height_) ? tempH - j - dy : j + dy)*kernel(dy + r, dx + r);
          }
        }

        u(i, j) = fx;
        v(i, j) = fy;
      }
    }
  }

  Matrix<double> Image::gradient(std::function<double(double, double)> value_in_point) const {
    Matrix<double> u, v;
    gradient(u, v);

    // воспользуемся `u` как результирующей матрицей 
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        u(i, j) = value_in_point(v(i, j), u(i, j));
      }
    }

    return u;
  }

  Matrix<double> Image::gradient(const Matrix<double>& kernel, std::function<double(double, double)> value_in_point) const {
    Matrix<double> u, v;
    gradient(kernel, u, v);

    // воспользуемся `u` как результирующей матрицей 
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        u(i, j) = value_in_point(v(i, j), u(i, j));
      }
    }

    return u;
  }

  Image& Image::erode(int radius) {
    Image old(*this);

    bool fl;
    for (int y = radius, n = height_ - radius; y < n; ++y) {
      for (int x = radius, m = width_ - radius; x < m; ++x) {
        if (old.byte(x, y) == 255) {
          fl = false;
          for (int i = -radius; i <= radius && !fl; ++i) {
            for (int j = -radius; j <= radius && !fl; ++j) {
              if (i == 0 && j == 0) continue;
              if (old.byte(x + i, y + j) != 255) fl = true;
            }
          }

          if (!fl) byte(x, y) = 255;
          else byte(x, y) = 0;
        }
      }
    }

    return *this;
  }

  Image& Image::dilate(int radius) {
    Image old(*this);
    for (int y = radius, n = height_ - radius; y < n; ++y) {
      for (int x = radius, m = width_ - radius; x < m; ++x) {
        if (old.byte(x, y) == 255) {
          for (int i = -radius; i <= radius; ++i) {
            for (int j = -radius; j <= radius; ++j) {
              byte(x + i, y + j) = 255;
            }
          }
        }
      }
    }

    return *this;
  }

  Image& Image::closing(int radius) {
    dilate(radius);
    return erode(radius);
  }

  Image& Image::opening(int radius) {
    erode(radius);
    return dilate(radius);
  }

  Image& Image::nonMaximumSuppression(const matd& directon) {
    int di, dj;
    uint8_t current;
    Image old(*this);
    for (int i = 0; i<width_; ++i) {
      for (int j = 0; j<height_; ++j) {
        current = old.byte(i, j);
        di = xr::math::sign((int)round(cos(directon(i, j))));
        dj = xr::math::sign((int)round(sin(directon(i, j))));
        if ((isCorrect(i + di, j + dj) && old.byte(i + di, j + dj) > current) ||
          (isCorrect(i - di, j - dj) && old.byte(i - di, j - dj) > current)) {
          byte(i, j) = 0;
        }
      }
    }

    return *this;
  }

  Image& Image::bilateralFiltering(double sigma_s, double sigma_r) { // Если радиус = 0, то используется радиус 2*sigma
    int radius = 2 * static_cast<int>(sigma_s);

    auto w = [&](int i, int j, int k, int l) -> double {
      double first = -((i - k)*(i - k) + (j - l)*(j - l)) * 0.5 / sigma_s / sigma_s;
      double second = -math::sqr(std::abs(byte(i, j) - byte(k, l))) * 0.5 / sigma_r / sigma_r;
      return exp(first + second);
    };

    auto src = to<double>();
    double factor = 0, color = 0, temp;
    for (int i = radius; i < width_ - radius; ++i) {
      for (int j = radius; j < height_ - radius; ++j) {
        color = 0;
        factor = 0;
        for (int dx = -radius; dx <= radius; dx++) {
          for (int dy = -radius; dy <= radius; dy++) {
            temp = w(i, j, i + dx, j + dy);
            color += src(i + dx, j + dy) * temp;
            factor += temp;
          }
        }

        byte(i, j) = static_cast<uint8_t>(std::round(color / factor));
      }
    }

    return *this;
  }

  Image& Image::gaussianBlur(int radius, double sigma) { // Если радиус = 0, то используется радиус 3*sigma
    if (radius == 0) {
      radius = static_cast<int>(std::round(3 * sigma));
    }

    from(convolution(Matrix<double>::makeGaussianKernel(radius, sigma, true)));
    return *this;
  }

  Image& Image::medianBlur(int radius) {
    *this = convolution(Matrix<double>::makeAveragingKernel(radius));
    return *this;
  }

  Image& Image::gaussianBlurForCanny() {
    from(convolution(matd::makeGaussianForCannyKernel()));
    return *this;
  }

  Image& Image::kuwahara(int radius) {
    Matrix<double> src(to<double>());

    int n;
    double medium[4];
    double variance[4];
    int dx[4] = {-1, 1, -1, 1};
    int dy[4] = {-1, -1, 1, 1};

    for (int i = 1; i < width_ - 1; ++i) {
      for (int j = 1; j < height_ - 1; ++j) {
        for (int k = 0; k < 4; ++k) {
          n = 0;
          variance[k] = medium[k] = 0;
          for (int ii = 0; abs(ii) <= radius; ii += dx[k]) {
            for (int jj = 0; abs(jj) <= radius; jj += dy[k]) {
              if (!isCorrect(i + ii, j + jj)) continue;
              variance[k] += src(i + ii, j + jj)*src(i + ii, j + jj);
              medium[k] += src(i + ii, j + jj);
              ++n;
            }
          }

          medium[k] /= n;
          variance[k] = 1.0 / n*variance[k] - medium[k] * medium[k];
        }

        int target = std::min_element(variance, variance + 4) - variance; //ptr. diff
        byte(i, j) = static_cast<uint8_t>(medium[target]);
      }
    }

    return *this;
  }

  Image& Image::laplace(int mode) {
    switch (mode) {
    case 4:
      *this = convolution(Matrix<double>::makeLaplace4Kernel()).scale(0.0, 255.0);
      break;
    case 8:
      *this = convolution(Matrix<double>::makeLaplace8Kernel()).scale(0.0, 255.0);
      break;
    case 12:
      *this = convolution(Matrix<double>::makeLaplace12Kernel()).scale(0.0, 255.0);
      break;
    }

    return *this;
  }

  Image& Image::sobel() {
    auto abs = [](double fy, double fx) -> double {
      return std::sqrt(fx*fx + fy*fy);
    };

    from(gradient(Matrix<double>::makeSobelKernel(), abs).scale(0.0, 255.0));
    return *this;
  }

  Image& Image::kirsch() {
    Matrix<double> dst(size(), 0.0);
    for (int i = 1; i < width_ - 1; ++i) {
      for (int j = 1; j < height_ - 1; ++j) {
        int f = 0;
        for (int ind = 0; ind < 8; ++ind) {
          int s = 0, t = 0;
          for (int k = 0; k < 3; ++k) {
            int index = xr::math::normalize(k + ind, 8);
            s += byte(i + xr::math::dx[index], j + xr::math::dy[index]);
          }

          for (int k = 3; k < 8; ++k) {
            int index = xr::math::normalize(k + ind, 8);
            t += byte(i + xr::math::dx[index], j + xr::math::dy[index]);
          }

          f = xr::math::max(f, abs(5 * s - 3 * t));
        }

        dst(i, j) = f;
      }
    }

    return from(dst.scale(0.0, 255.0));
  }

  Image& Image::clear(const uint8_t& val) {
    uint8_t* cur = data_;
    for (int i = 0, n = width_*height_; i < n; ++i) {
      *cur++ = val;
    }

    return *this;
  }

  Image& Image::histogramEqualize() {
    std::vector<int> hist = histogram<int>();
    std::vector<double> table(256);

    int sum = hist[0];
    table[0] = double(hist[0]);
    for (int i = 1; i < 256; ++i) {
      sum += hist[i];
      table[i] = double(sum);
    }

    std::vector<uint8_t> LUT(256);
    for (int i = 0; i < 256; ++i) {
      LUT[i] = uint8_t(round(255.0*table[i] / sum));
    }

    uint8_t* cur = data_;
    for (int i = 0, n = width_*height_; i < n; ++i) {
      *cur = LUT[*cur];
      ++cur;
    }

    return *this;
  }

  Image& Image::contrast() {
    Matrix<double> kernel(3, 3, 0.0);
    kernel(1, 1) = 5;
    kernel(0, 1) = -1;
    kernel(1, 0) = -1;
    kernel(2, 1) = -1;
    kernel(1, 2) = -1;

    from(convolution(kernel));

    return *this;
  }

  Image& Image::invert() {
    uint8_t* cur = data_;
    int pixels = width_ * height_;
    for (int i = 0; i < pixels; ++i) {
      *cur = 255 - *cur;
      ++cur;
    }

    return *this;
  }

  Image& Image::hFlip() {
    for (int j = 0; j < height_; ++j) {
      for (int i = 0, w = width_ / 2; i <= w; ++i) {
        std::swap(byte(i, j), byte(width_ - 1 - i, j));
      }
    }

    return *this;
  }

  Image& Image::vFlip() {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0, h = height_ / 2; j <= h; ++j) {
        std::swap(byte(i, j), byte(i, height_ - 1 - j));
      }
    }

    return *this;
  }

  Image& Image::trim(int margin) {
    Image temp(width_ - margin * 2, height_ - margin * 2);
    for (int i = 0; i < temp.width_; ++i) {
      for (int j = 0; j < temp.height_; ++j) {
        temp.byte(i, j) = byte(i + margin, j + margin);
      }
    }

    return swap(temp);
  }

  Image& Image::trim(uint8_t color) {
    rect_t rect(width_ - 1, 0, height_ - 1, 0);

    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        if (byte(i, j) != color) {
          rect.left = math::min(i, rect.left);
          rect.right = math::max(i, rect.right);
          rect.bottom = math::min(j, rect.bottom);
          rect.top = math::max(j, rect.top);
        }
      }
    }

    if (rect.left >= rect.right || rect.bottom >= rect.top) {
      release();
    }
    else {
      Image temp(rect.right - rect.left + 1, rect.top - rect.bottom + 1);
      for (int i = rect.left; i <= rect.right; ++i) {
        for (int j = rect.bottom; j <= rect.top; ++j) {
          temp.byte(i - rect.left, j - rect.bottom) = byte(i, j);
        }
      }

      swap(temp);
    }

    return *this;
  }

  Image& Image::setFrame(int border_width, uint8_t color) {
    for (int k = 0; k < border_width; k++) {
      for (int i = 0; i < width_; ++i) {
        byte(i, k) = color;
        byte(i, height_ - 1 - k) = color;
      }

      for (int i = 0; i < height_; ++i) {
        byte(k, i) = color;
        byte(width_ - 1 - k, i) = color;
      }
    }

    return *this;
  }

  Image& Image::changeColor(uint8_t old_color, uint8_t new_color) {
    uint8_t* cur = data_;
    int pixels = width_*height_;
    for (int i = 0; i < pixels; ++i) {
      if (*cur == old_color) *cur = new_color;
      ++cur;
    }

    return *this;
  }

  Image& Image::changeColor(bool(*pred)(uint8_t), uint8_t new_color) {
    uint8_t* cur = data_;
    int pixels = width_*height_;
    for (int i = 0; i < pixels; ++i) {
      if (pred(*cur)) *cur = new_color;
      ++cur;
    }

    return *this;
  }

  Image& Image::turnClockwise() {
    Image dst(height(), width());
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        dst.byte(j, i) = byte(i, j);
      }
    }

    return swap(dst);
  }

  Image& Image::turnCounterClockwise() {
    return turnClockwise().hFlip();
  }

  Image Image::subimage(int x, int y, int width, int height) {
    Image dst(width, height);
    for (int i = 0; i < width; ++i) {
      for (int j = 0; j < height; ++j) {
        dst.byte(i, j) = byte(i + x, j + y);
      }
    }

    return dst;
  }

  Image Image::subimage(const rect_t& roi) {
    return std::move(subimage(roi.left, roi.bottom, roi.width(), roi.height()));
  }

  Image& Image::floodFill(int x, int y, uint8_t color, xr::Connectivity way) {
    std::stack<point_t> stack;
    stack.emplace(x, y);

    uint8_t field_color = byte(x, y);
    byte(x, y) = color;
    do {
      auto cur = stack.top();
      stack.pop();
      for (int i = 0; i < way; ++i) {
        point_t tmp(cur.x + math::dx[i], cur.y + math::dy[i]);
        if (isCorrect(tmp) && byte(tmp) == field_color) {
          stack.push(tmp);
          byte(tmp) = color;
        }
      }
    } while (!stack.empty());

    return *this;
  }

  Image& Image::fillSmallAreas(size_t max_region_size) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        if (byte(i, j) == 0) {
          auto points = getPointsRegion(i, j, Connectivity::Four);
          if (points.size() < max_region_size) {
            for (auto it : points) byte(it) = 255;
          }
          else {
            for (auto it : points) byte(it) = 1; //временно
          }
        }
      }
    }

    return changeColor(1, 0);
  }

  points_t Image::getPointsRegion(int x, int y, xr::Connectivity way, int upper_limit) const {
    std::stack<point_t, points_t> stack;
    Matrix<bool> used(size(), false);
    points_t dst;

    dst.reserve(width_*height_ / 6);
    stack.emplace(x, y);
    used(x, y) = true;
    uint8_t color = byte(x, y);
    do {
      auto cur = stack.top();
      dst.push_back(cur);
      stack.pop();
      for (int i = 0; i < way; ++i) {
        point_t tmp(cur.x + math::dx[i], cur.y + math::dy[i]);
        if (isCorrect(tmp) && byte(tmp) == color && !used(tmp)) {
          stack.push(tmp);
          used(tmp) = true;
        }
      }
    } while (!stack.empty() && (int)dst.size() < upper_limit);

    return dst;
  }

  void Image::gvf(double mu, int iters, Matrix<double>& u, Matrix<double>& v) {
    Matrix<double> f = to<double>();
    f.scale(0, 1);

    u.recreate(f.width(), f.height(), 0.0);
    v.recreate(f.width(), f.height(), 0.0);

    /* Compute derivative */
    for (int i = 1; i < width() - 1; ++i) {
      for (int j = 1; j < height() - 1; ++j) {
        u(i, j) = 0.5*(f(i + 1, j) - f(i - 1, j));
        v(i, j) = 0.5*(f(i, j + 1) - f(i, j - 1));
      }
    }

    for (int j = 0; j < height(); ++j) {
      u(0, j) = 0.5*(f(1, j) - f(0, j));
      u(width() - 1, j) = 0.5*(f(width() - 1, j) - f(width() - 2, j));
    }

    for (int i = 0; i < width(); ++i) {
      v(i, 0) = 0.5*(f(i, 1) - f(i, 0));
      v(i, height() - 1) = 0.5*(f(i, height() - 1) - f(i, height() - 2));
    }

    /* Compute parameters and initializing arrays */
    Matrix<double> b(f.size()), c1(f.size()), c2(f.size());
    for (int i = 0; i < width(); ++i) {
      for (int j = 0; j < height(); ++j) {
        b(i, j) = math::sqr(u(i, j)) + math::sqr(v(i, j));
        c1(i, j) = b(i, j)*u(i, j);
        c2(i, j) = b(i, j)*v(i, j);
      }
    }

    /* Solve GVF = (u,v) */
    Matrix<double> Lu(size()), Lv(size());
    for (int it = 0; it < iters; ++it) {
      /* corners */
      int n = width() - 1;
      int m = height() - 1;
      Lu(0, 0) = (2 * u(1, 0) + 2 * u(0, 1)) - 4 * u(0, 0);
      Lv(0, 0) = (2 * v(1, 0) + 2 * v(0, 1)) - 4 * v(0, 0);
      Lu(n, m) = (2 * u(n - 1, m) + u(n, m - 1)) - 4 * u(n, m);
      Lv(n, m) = (2 * v(n - 1, m) + v(n, m - 1)) - 4 * v(n, m);
      Lu(n, 0) = (2 * u(n - 1, 0) + 2 * u(n, 1)) - 4 * u(n, 0);
      Lv(n, 0) = (2 * v(n - 1, 0) + 2 * v(n, 1)) - 4 * v(n, 0);
      Lu(0, m) = (2 * u(1, m) + 2 * u(0, m - 1)) - 4 * u(0, m);
      Lv(0, m) = (2 * v(1, m) + 2 * v(0, m - 1)) - 4 * v(0, m);

      /* interior Lu, Lv*/
      double* uCur, *uPrev, *uNext;
      double* vCur, *vPrev, *vNext;
      double* curLu = Lu.data();
      double* curLv = Lv.data();
      for (int j = 1; j < m; ++j) {
        uCur = u.line(j) + 1;
        uPrev = u.line(j - 1) + 1;
        uNext = u.line(j + 1) + 1;
        vCur = v.line(j) + 1;
        vPrev = v.line(j - 1) + 1;
        vNext = v.line(j + 1) + 1;
        curLu = Lu.line(j) + 1;
        curLv = Lv.line(j) + 1;
        for (int i = 1; i < n; ++i) {
          *curLu++ = (*(uCur - 1) + *uPrev++ + *(uCur + 1) + *uNext++) - 4 * (*uCur);
          *curLv++ = (*(vCur - 1) + *vPrev++ + *(vCur + 1) + *vNext++) - 4 * (*vCur);
          ++uCur;
          ++vCur;
        }
      }
      /* left and right columns */
      for (int j = 1; j < m; ++j) {
        Lu(0, j) = (u(0, j - 1) + 2 * u(1, j) + u(0, j + 1)) - 4 * u(0, j);
        Lv(0, j) = (v(0, j - 1) + 2 * v(1, j) + v(0, j + 1)) - 4 * v(0, j);
        Lu(n, j) = (u(n, j - 1) + 2 * u(n - 1, j) + u(n, j + 1)) - 4 * u(n, j);
        Lv(n, j) = (v(n, j - 1) + 2 * v(n - 1, j) + v(n, j + 1)) - 4 * v(n, j);
      }

      /* top and bottom rows */
      for (int i = 1; i < n; ++i) {
        Lu(i, 0) = (u(i - 1, 0) + 2 * u(i, 1) + u(i + 1, 0)) - 4 * u(i, 0);
        Lv(i, 0) = (v(i - 1, 0) + 2 * v(i, 1) + v(i + 1, 0)) - 4 * v(i, 0);
        Lu(i, m) = (u(i - 1, m) + 2 * u(i, m - 1) + u(i + 1, m)) - 4 * u(i, m);
        Lv(i, m) = (v(i - 1, m) + 2 * v(i, m - 1) + v(i + 1, m)) - 4 * v(i, m);
      }

      /* Update GVF  */
      double* curU = u.data();
      double* curV = v.data();
      double* curb = b.data();
      double* curC1 = c1.data();
      double* curC2 = c2.data();
      curLu = Lu.data();
      curLv = Lv.data();
      for (int i = 0, n = width()*height(); i < n; ++i) {
        *curU = (1.0 - *curb) * (*curU) + mu * (*curLu++) + *curC1++;
        *curV = (1.0 - *curb) * (*curV) + mu * (*curLv++) + *curC2++;
        ++curU;
        ++curV;
        ++curb;
      }
    }
  }

  Image Image::gvf(double mu, int iters, std::function<double(double, double)> unite_func) {
    Matrix<double> u(size()), v(size());
    gvf(mu, iters, u, v);

    return Matrix<double>::unite(u, v, unite_func);
  }

  /* others */
  Image imread(const std::string& filename) {
    cv::Mat src = cv::imread(filename);
    if (src.channels() > 1) {
      cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
    }

    Image ans(src.cols, src.rows);
    int pixels = src.cols * src.rows;
    uint8_t* cur_dst = ans.data();
    uint8_t* cur_src = src.data;
    for (int i = 0; i < pixels; ++i) {
      *cur_dst++ = *cur_src++;
    }

    return ans;
  }

  bool imwrite(const Image& image, const std::string& filename) {
    cv::Mat sample(cv::Size(image.width(), image.height()), CV_8UC1, image.data());
    return cv::imwrite(filename, sample);
  }

  Image* draw(Image* image, const contour_t& contour, uint8_t color) {
    for (auto &e : contour) {
      if (image->isCorrect(e)) {
        image->byte(e) = color;
      }
      else std::cout << "incorrect pixel index: " << e.x << " " << e.y << std::endl;
    }

    return image;
  }

  Image* draw(Image* image, const contours_t& contours, uint8_t color) {
    for (auto& contour : contours) {
      for (auto &e : contour) {
        image->byte(e) = color;
      }
    }

    return image;
  }
}
