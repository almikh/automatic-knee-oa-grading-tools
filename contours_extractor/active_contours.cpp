#include "active_contours.h"
#include "utility.h"
#include <iostream>

namespace xr
{
  ActiveContours::ActiveContours(Data::HardPtr data) :
    data_(data),
    simplification_degree_(5),
    gradient_ref_(&data->gradient),
    path_finder_(data->initial.size()),
    enable_uniform_points_distribution_(false)
  {
    energies_[Energy::Image] = -0.1;
    energies_[Energy::Gradient] = -10.0;
    energies_[Energy::GradientDir] = 1.0;
    energies_[Energy::Expanse] = -0.07;
    energies_[Energy::Contour] = 0.05;

    path_finder_.setGradientRef(gradient_ref_);
    path_finder_.setPassability(std::make_shared<AllTraversable>());
  }
  
  void ActiveContours::setGradientRef(Matrix<double>* grad_ref) {
    gradient_ref_ = grad_ref;
    path_finder_.setGradientRef(gradient_ref_);
  }

  void ActiveContours::enableUniformPointsDistribution(bool enable) {
    enable_uniform_points_distribution_ = enable;
  }

  void ActiveContours::setSimplificationDegree(int points) {
    simplification_degree_ = points;
  }

  void ActiveContours::setEnergy(Energy energy, double value) {
    energies_[energy] = value;
  }

  contour_t& ActiveContours::run(contour_t& contour, int radius, int max_iters) {
    // TODO попробовать обойтись без создания image_dbl
    matd image_dbl(std::move(data_->initial.to<double>().scale(0.0, 1.0)));
    gradient_ref_->scale(0.0, 1.0);

    size_t begin_size = contour.size();
    auto working_copy = simplify(contour, simplification_degree_);

    // энергии
    double e_img = energies_[Energy::Image];
    double e_grad = energies_[Energy::Gradient];
    double e_dir = energies_[Energy::GradientDir];
    double e_cont = energies_[Energy::Contour];
    double e_exp = energies_[Energy::Expanse];

    int counter, changed;
    point<double> normal;
    point_t prev, next, cur, jk;
    double temp1, temp2, temp3, temp4, d_temp1, d_temp2;
    for (counter = 0, changed = 1; counter < max_iters && changed; ++counter) {
      changed = 0; // будем считать изменения

      auto n = working_copy.size();
      double gamma = 0.5 / (std::cos(2 * math::Pi / n));
      double lV = working_copy[0].sqrDist(working_copy[n - 1]);
      for (size_t k = 1; k < n; ++k) {
        lV = working_copy[k].sqrDist(working_copy[k - 1]);
      }

      lV /= n;

      for (size_t i = 0; i < n; ++i) {
        prev = working_copy[(i==0) ? n - 1 : i - 1];
        next = working_copy[(i + 1) % n];
        cur = working_copy[i];

        temp1 = cur.x - prev.x;
        temp2 = cur.y - prev.y;
        temp3 = next.x - cur.x;
        temp4 = cur.y - prev.y;
        d_temp1 = sqrt(math::sqr(temp1) + math::sqr(temp2));
        d_temp2 = sqrt(math::sqr(temp3) + math::sqr(temp4));
        normal = make_point(-temp2 / d_temp1 - temp4 / d_temp2, temp1 / d_temp1 + temp3 / d_temp2);

        point_t min_el(0, 0);
        double min_val = Double::max();
        for (int j = -radius; j <= radius; ++j) {
          for (int k = -radius; k <= radius; k++) {
            jk = make_point(cur.x + j, cur.y + k);
            if (!data_->initial.isCorrect(jk)) continue;

            double jk_val =
              + e_img * image_dbl(jk)
              + e_grad * gradient_ref_->at(jk)
              + e_dir * math::dirDist(data_->gradient_dir(jk), data_->gradient_dir(cur)) / math::Pi
              + e_cont / lV * (math::sqr(jk.x - gamma*(prev.x + next.x)) + math::sqr(jk.y - gamma*(prev.y + next.y)))
              + e_exp * (normal.x*(cur.x - jk.x) + normal.y*(cur.y - jk.y));

            if (jk_val < min_val) {
              min_val = jk_val;
              min_el = make_point(j, k);
            }
          }
        }

        if (min_el.x || min_el.y) {
          ++changed;
          working_copy[i] += min_el;
        }
      }

      if (enable_uniform_points_distribution_) {
        amplify(working_copy, &path_finder_);
        simplify(working_copy, simplification_degree_);
      }
    }

    int prev_size = working_copy.size();
    unique(working_copy);

    const size_t min_contour_length = 10;
    if ((prev_size / working_copy.size() < 2) && (working_copy.size() > min_contour_length)) {
      contour = std::move(amplify(working_copy, &path_finder_));
      if ((contour.size()>begin_size * 2) || (contour.size()*1.5 < begin_size)) {
        contour.clear();
      }
    }
    else contour.clear();

    return contour;
  }
}
