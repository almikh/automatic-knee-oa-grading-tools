#include <assert.h>
#include <iostream>
#include <algorithm>
#include <omp.h>

#include "multithreaded_threshold_finder.h"
#include "gaps_remover.h"
#include "simple_contours_finder.h"
#include "dev_contours_finder.h"
#include "contours_finder.h"
#include "analysis.h"

namespace xr
{
  MultithreadedThresholdFinder::MultithreadedThresholdFinder(Data::HardPtr data):
    ThresholdFinder(data)
  {

  }

  uint8_t MultithreadedThresholdFinder::find(uint8_t approximation, double step, int count) {
    target_index_ = -1;
    buffer_.clear();

    using target_t = std::tuple<Verifier::HardPtr, std::shared_ptr<Image>, uint8_t>;

    // просчитаем пороги
    std::vector<target_t> targets;
    for (int ind = 1; ind<count; ++ind) {
      auto current_threshold = static_cast<uint8_t>(approximation*ind*step);
      if (targets.empty() || current_threshold != std::get<2>(targets.back())) {
        ContoursFinder::HardPtr contours_finder = std::make_shared<DevContoursFinder>(data_);

        auto clone = std::make_shared<Image>(std::move(data_->working.clone()));
        auto verifier = std::make_shared<Verifier>(data_, binary_ver_);
        verifier->setContoursFinder(contours_finder);

        targets.push_back(std::make_tuple(verifier, clone, current_threshold));
      }
    }

    // поиск
    buffer_.resize(targets.size());
    double max_valuation = Double::max();
    int size = static_cast<int>(targets.size());

#pragma omp parallel
    {
#pragma omp for
    for (int i = 0; i < size; ++i) {
      auto verifier = std::get<0>(targets[i]);
      auto image = std::get<1>(targets[i]);
      auto threshold = std::get<2>(targets[i]);

      buffer_[i] = verifier->verify(image.get(), threshold);
      buffer_[i].image = image;

      std::cout << omp_get_thread_num() << "> " << (int)buffer_[i].threshold << " [" << buffer_[i].valuation << "]" << 
        " " << image->sum() << std::endl;
    }
    }

    auto target_it = std::max_element(buffer_.begin(), buffer_.end(), [](const Item& f, const Item& s) {
      return f.valuation < s.valuation;
    });

    target_index_ = target_it - buffer_.begin();
    return buffer_[target_index_].threshold;
  }
}
