#pragma once
#include <memory>
#include "image.h"
#include "matrix.h"
#include "xr_math.h"

namespace xr
{
  struct Data {
    using HardPtr = std::shared_ptr<Data>;

    Image initial;
    Image working; // с этим как раз работаем
    Matrix<double> gradient;
    Matrix<double> gradient_dir;
    uint8_t otsu_threshold; // порог по Отсу для исходного изображения

    Data(Image&& source);

    void prepare();
  };
}
