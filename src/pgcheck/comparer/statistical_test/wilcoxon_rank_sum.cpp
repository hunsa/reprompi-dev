/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#include "wilcoxon_rank_sum.h"

double WilcoxonRankSum::get_z_value() {
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
    if (i < n && (j >= m || sample_1[i] <= sample_2[j])) {
      sample_1_rank_sum += i + j + 1;
      ++i;
    } else {
      sample_2_rank_sum += i + j + 1;
      ++j;
    }
  }

  // assert that all ranks where assigned
  assert((sample_1_rank_sum + sample_2_rank_sum) == (size_sum * (size_sum + 1) / 2));

  double standard_deviation = std::sqrt(size_squared * (size_sum + 1) / 12);
  z_value = (sample_2_rank_sum - get_expected_value()) / standard_deviation;
  return z_value;
}

double WilcoxonRankSum::get_critical_value() {
  int size = std::min(sample_1.size(), sample_2.size());
  return TwoSampleTest::get_critical_value(size);
}

double WilcoxonRankSum::get_shared_rank_deviation(int shared_rank_count) {
  return (std::pow(shared_rank_count, 3) - shared_rank_count) / 2;
}

double WilcoxonRankSum::get_expected_value() {
  return sample_1.size() * (sample_2.size() + sample_2.size() + 1) / 2;
}

bool WilcoxonRankSum::get_violation() {
  if (z_value != 0) {
    return z_value < -get_critical_value();
  } else {
    return get_z_value() < -get_critical_value();
  }
}

std::vector <std::pair<double, bool>> WilcoxonRankSum::get_ordered_diff_sign_vector() {
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
