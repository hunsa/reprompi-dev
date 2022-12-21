//
// Created by Max on 10/30/22.
//

#include "wilcoxon.h"

double Wilcoxon::get_z_value() {

  std::sort(sample_1.begin(), sample_1.end());
  std::sort(sample_2.begin(), sample_2.end());
  long sample_1_rank_sum = 0;
  long sample_2_rank_sum = 0;
  long size_sum = sample_1.size() + sample_2.size();
  long current_rank = 1;
  size_t i = 0;
  size_t j = 0;
  while (current_rank <= size_sum) {
    if (sample_1[i] <= sample_2[j]) {
      sample_1_rank_sum += current_rank;
      if (i < sample_1.size()) {
        i++;
      }
    } else {
      sample_2_rank_sum += current_rank;
      if (j < sample_2.size()) {
        j++;
      }
    }
    current_rank++;
  }

  long rank_sum = sample_1_rank_sum + sample_2_rank_sum;
  long rank_check_sum = ((sample_1.size() + sample_2.size()) * (sample_1.size() + sample_2.size() + 1)) / 2;

  assert(rank_sum == rank_check_sum);

  double size_squared = sample_1.size() * sample_2.size();
  double u_1 = size_squared + sample_1.size() * (1 + sample_1.size()) / 2 - sample_1_rank_sum;
  double u_2 = size_squared + sample_2.size() * (1 + sample_2.size()) / 2 - sample_2_rank_sum;

  double u_sum = (u_1 + u_2);
  double u_check_sum = size_squared;

  assert(u_sum == u_check_sum);

  double wilcoxon_value = std::min(u_1, u_2);
  double standard_deviation = sample_1.size() * sample_2.size() * (1 + size_sum);
  standard_deviation = standard_deviation / 12;
  standard_deviation = std::sqrt(standard_deviation);
  z_value = (wilcoxon_value - get_expected_value()) / standard_deviation;
  return z_value;
}

double Wilcoxon::get_critical_value() {
  int size = std::min(sample_1.size(), sample_2.size());
  return TwoSampleTest::get_critical_value(size);
}

double Wilcoxon::get_shared_rank_deviation(int shared_rank_count) {
  return (std::pow(shared_rank_count, 3) - shared_rank_count) / 2;
}

double Wilcoxon::get_expected_value() {
  return (sample_1.size() + sample_2.size()) / 2;
}

bool Wilcoxon::get_violation() {
  if (z_value != 0) {
    return z_value < -get_critical_value();
  } else {
    return get_z_value() < -get_critical_value();
  }
}

std::vector <std::pair<double, bool>> Wilcoxon::get_ordered_diff_sign_vector() {
  int size = std::min(sample_1.size(), sample_2.size());
  std::vector <std::pair<double, bool>> diff_vector;
  diff_vector.push_back(std::make_pair(0, false));
  for (int idx = 0; idx < size; idx++) {
    bool sign = false;
    double diff = sample_1[idx] - sample_2[idx];
    if (diff < 0) {
      sign = true;
    }
    diff_vector.push_back(std::make_pair(std::abs(diff), sign));
  }
  std::sort(diff_vector.begin(), diff_vector.end());
  return diff_vector;
}