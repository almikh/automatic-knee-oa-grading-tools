#include "session.h"

namespace xr
{
  Data::Data(Image&& source) :
    initial(source) 
  {
    otsu_threshold = initial.thresholdByOtsu();
    working = std::move(initial.clone());
  }

  void Data::prepare() {
    Matrix<double> u, v;
    working.gradient(Matrix<double>::makeSobelKernel(), u, v);
    gradient = std::move(Matrix<double>::unite(u, v, math::grad::abs));
    gradient_dir = std::move(Matrix<double>::unite(u, v, math::grad::dirInRad));
  }
}
