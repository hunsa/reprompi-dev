//
// Created by Max on 12/27/22.
//

#include "wilcoxon_mann_whitney.h"

double WilcoxonMannWhitney::get_z_value() {

  double n = sample_1.size();
  double m = sample_2.size();
  double size_squared = n * m;
  double size_sum = n + m;

  std::size_t i = 0;
  std::size_t j = 0;

  double sample_1_rank_sum = 0;
  double sample_2_rank_sum = 0;

  std::sort(sample_1.begin(), sample_1.end());
  std::sort(sample_2.begin(), sample_2.end());

  while (i < n || j < m) {
    if (i < n && (j >= m || sample_1[i] < sample_2[j])) {
      sample_1_rank_sum += i + j + 1;
      ++i;
    } else {
      sample_2_rank_sum += i + j + 1;
      ++j;
    }
  }

  // assert that all ranks where assigned
  assert((sample_1_rank_sum + sample_2_rank_sum) == (size_sum * (size_sum + 1) / 2));

  // assert that u calculation was successful
  double U = size_squared + n * (n + 1) / 2 - sample_1_rank_sum;
  assert((U + size_squared + m * (m + 1) / 2 - sample_2_rank_sum) == size_squared);

  double standard_deviation = std::sqrt(size_squared * (size_sum + 1) / 12);
  z_value = (U - get_expected_value()) / standard_deviation;
  return z_value;
}

double WilcoxonMannWhitney::get_critical_value() {
  int size = std::min(sample_1.size(), sample_2.size());
  return TwoSampleTest::get_critical_value(size);
}

double WilcoxonMannWhitney::get_shared_rank_deviation(int shared_rank_count) {
  return (std::pow(shared_rank_count, 3) - shared_rank_count) / 2;
}

double WilcoxonMannWhitney::get_expected_value() {
  return sample_1.size() * sample_2.size() / 2;
}

bool WilcoxonMannWhitney::get_violation() {
  if (z_value != 0) {
    return z_value < -get_critical_value();
  } else {
    return get_z_value() < -get_critical_value();
  }
}