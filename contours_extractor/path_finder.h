#pragma once
#include <memory>
#include "defs.h"
#include "size.h"
#include "point.h"
#include "matrix.h"
#include "image.h"

namespace xr
{
  struct PassabilityMap {
    using HardPtr = std::shared_ptr<PassabilityMap>;
    virtual bool isTraversable(const point_t&) = 0;
  };

  struct AllTraversable : public PassabilityMap {
    using HardPtr = std::shared_ptr<AllTraversable>;
    bool isTraversable(const point_t&) override {
      return true;
    }
  };

  class Bitmap : public PassabilityMap {
    Image* source_;
  public:
    using HardPtr = std::shared_ptr<AllTraversable>;

    Bitmap(Image* source) : source_(source) {}

    bool isTraversable(const point_t& p) override {
      return source_->byte(p) != 0;
    }
  };

  // TODO направление градиента
  class PathFinder {
    int label_ = 0;
    double factor_ = 0.0;
    double medium_ = 0.0;
    path_t last_path_;
    Matrix<double> grad_;
    Matrix<int> label_map_;
    Matrix<double>* grad_ref_ = nullptr;
    Matrix<double> price_, aux_price_;
    PassabilityMap::HardPtr pass_map_;

  public:
    using HardPtr = std::shared_ptr<PathFinder>;

    enum Mode {
      Distance = 1,
      Gradient = 2,
      GradientDiff = 4
    };

    PathFinder();
    PathFinder(const Size& size);

    bool find(point_t first, point_t last, int flag = Distance | Gradient, bool include_init_points = false);
    path_t lastPath() const;

    void setGradientRef(Matrix<double>* gradient);
    void setGradient(const Matrix<double>& gradient);
    void setPassability(PassabilityMap::HardPtr map);
    void scaleGradient(double down, double up);
    void setFactor(double factor);
  };
}
