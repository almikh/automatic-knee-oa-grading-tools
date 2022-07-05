#include <iostream>
#include "simple_contours_finder.h"
#include "simple_key_points_finder.h"
#include "gaps_remover.h"
#include "analysis.h"
#include "utility.h"

namespace xr
{
  SimpleContoursFinder::SimpleContoursFinder(Data::HardPtr data) :
    ContoursFinder(data) {
    path_finder_ = std::make_shared<PathFinder>(data_->initial.size());
    path_finder_->setGradientRef(&data_->gradient);
    path_finder_->scaleGradient(0.0, 255.0);
    path_finder_->setFactor(0.0);
  }

  contours_t SimpleContoursFinder::find(Image* image, SearchMode mode, int threshold) {
    path_finder_->setPassability(std::make_shared<Bitmap>(image));

    // ищем контуры в бинарном изображении
    regions_t regions;
    mati marked = colorize(*image, data_->initial, &regions);

    data_->gradient.scale(0, 255);
    SimpleKeyPointsFinder key_points_finder(data_);

    contours_t contours;
    for (auto& region : regions) {
      bool suitable = (mode == SearchMode::FilterOut && region.medium_color > threshold);
      suitable |= (mode == SearchMode::All);
      if (suitable) {
        ParamsMap params;
        params[SimpleKeyPointsFinder::ParamName::Points].ivalue = 10;
        points_t key_points = key_points_finder.find(region, &marked, params);
        contour_t contour = initialContour(key_points);
        try {
          contour = toFundamental(contour);
        }
        catch (const std::exception& e) {
          std::cerr << e.what();
        }

        if ((mode == SearchMode::FilterOut && contour.size()>128) || mode == SearchMode::All) {
          contours.push_back(contour);
        }
      }
    }

    return contours;
  }
}
