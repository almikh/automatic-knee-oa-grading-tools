#include <string>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include "threshold_finder.h"
#include "gaps_remover.h"
#include "simple_contours_finder.h"
#include "dev_contours_finder.h"
#include "contours_finder.h"
#include "analysis.h"

namespace xr
{
  /* ThresholdFinder::Verifier */
  void ThresholdFinder::Verifier::removalDiscontinuities() {
    GapsRemover remover(data_, working_copy_);

    size_t size = 1;
    working_copy_->setFrame(1, 255);
    working_copy_->closing(1);

    double factors[] = {1.5, 1.25, 1.5, 1.0, 1.75, 1.75};
    int lenghts[] = {8 * size, 16 * size, 32 * size, 16 * size, 48 * size, 32 * size};

    remover.pathFinder()->scaleGradient(0.0, 10.0);
    for (int i = 0; i<3; ++i) {
      remover.pathFinder()->setFactor(factors[i]);
      remover.setMaxLength(lenghts[i]);
      remover.runMain(&BreakPointsDetector::is1st2nd3rdType);
    }

    auto isBoundary = [](Image* img, int x, int y)->bool {
      return x == 0 || x == img->width() - 1 || y == 0 || y == img->height() - 1;
    };
    auto isWhite = [](Image* img, int x, int y)->bool {
      return img->byte(x, y) == 255;
    };

    remover.pathFinder()->scaleGradient(0, 255);
    for (int i = 3; i<6; ++i) {
      remover.pathFinder()->setFactor(factors[i]);
      remover.setMaxLength(lenghts[i]);

      if (i != 5) remover.runAuxiliary(isBoundary);
      else remover.runAuxiliary(isWhite);
    }

    working_copy_->fillSmallAreas(16 * 16);
  }

  ThresholdFinder::Verifier::Verifier(Data::HardPtr data, std::shared_ptr<Image> binary):
    data_(data),
    binary_ver_(binary)
  {

  }
  
  void ThresholdFinder::Verifier::setContoursFinder(ContoursFinder::HardPtr finder) {
    contours_finder_ = finder;
  }

  ThresholdFinder::Item ThresholdFinder::Verifier::verify(Image* image, uint8_t threshold) {
    working_copy_ = image;

    ThresholdFinder::Item dst;
    dst.threshold = threshold;
    //dst.image = image;

    working_copy_->binarization(threshold);
    removalDiscontinuities();

    regions_t regions;
    uint8_t otsu = data_->otsu_threshold;
    mati marked = colorize(*working_copy_, data_->initial, &regions);

    auto mode = ContoursFinder::SearchMode::All;
    dst.contours = contours_finder_->find(working_copy_, mode, otsu);

    auto& edges = data_->gradient;
    Report report = createReport(marked, *binary_ver_, dst.contours, edges, otsu, regions);

    auto metric = metrics::COVERED_AND_UNCOVERED_AREA_WP;
    auto evaluation = calcMetricsQualityAllocation(report, metric) * report.wtw;
    dst.valuation = evaluation;

    return dst;
  }

  /* ThresholdFinder */
  ThresholdFinder::ThresholdFinder(Data::HardPtr data):
    data_(data)
  {
    binary_ver_.reset(new Image(std::move(data_->initial.clone())));
    binary_ver_->binarization(data_->otsu_threshold);
  }

  uint8_t ThresholdFinder::find(uint8_t approximation, double step, int count) {
    target_index_ = -1;
    buffer_.clear();

    using target_t = std::tuple<Verifier::HardPtr, std::shared_ptr<Image>, uint8_t>;

    // TODO SimpleContoursFinder -> DevContoursFinder
    ContoursFinder::HardPtr contours_finder = std::make_shared<DevContoursFinder>(data_);

    // просчитаем пороги
    std::vector<target_t> targets;
    for (int ind = 1; ind<count; ++ind) {
      auto current_threshold = static_cast<uint8_t>(approximation*ind*step);
      if (targets.empty() || current_threshold != std::get<2>(targets.back())) {
        auto clone = std::make_shared<Image>(std::move(data_->working.clone()));
        auto verifier = std::make_shared<Verifier>(data_, binary_ver_);
        verifier->setContoursFinder(contours_finder);

        targets.push_back(std::make_tuple(verifier, std::move(clone), current_threshold));
      }
    }

    // поиск
    double max_valuation = Double::max();
    int size = static_cast<int>(targets.size());

    for (int i = 0; i<size; ++i) {
      auto verifier = std::get<0>(targets[i]);
      auto image = std::get<1>(targets[i]);
      auto threshold = std::get<2>(targets[i]);

      auto item = verifier->verify(image.get(), threshold);
      item.image = image;
      buffer_.push_back(item);

      std::cout << "> " << (int)buffer_[i].threshold << " [" << buffer_[i].valuation << "]" <<
        " " << image->sum() << std::endl;
    }

    auto target_it = std::max_element(buffer_.begin(), buffer_.end(), [](const Item& f, const Item& s) {
      return f.valuation < s.valuation;
    });

    target_index_ = target_it - buffer_.begin();
    return buffer_[target_index_].threshold;
  }

  ThresholdFinder::Item& ThresholdFinder::goodImage() {
    assert(target_index_ >= 0);
    return buffer_[target_index_];
  }
}
