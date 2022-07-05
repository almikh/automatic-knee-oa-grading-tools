#pragma once
#include "defs.h"
#include "session.h"
#include "path_finder.h"

namespace xr
{
  class ActiveContours {
  public:
    enum Energy : int {
      Image = 0,
      Gradient,
      GradientDir,
      Expanse,
      Contour,
      Size // размер перечисления
    };

  protected:
    Data::HardPtr data_;
    PathFinder path_finder_;
    int simplification_degree_;
    Matrix<double>* gradient_ref_;
    bool enable_uniform_points_distribution_;
    double energies_[Energy::Size];

  public:
    ActiveContours(Data::HardPtr data);

    // поле градиента, используемое модулем (по умолчанию - градиента из data_)
    void setGradientRef(Matrix<double>* grad_ref);

    // на каждой итерации процесса адаптации контура 
    // будет происходить равномерное перераспределение точек
    // (во избежание слияния нескольких точек в одну и т.п.)
    void enableUniformPointsDistribution(bool enable);

    // при прореживании контура будет сохраняться каждая points-тая точка
    void setSimplificationDegree(int points);

    void setEnergy(Energy energy, double value);

    virtual contour_t& run(contour_t& contour, int radius, int max_iters);
  };
}
