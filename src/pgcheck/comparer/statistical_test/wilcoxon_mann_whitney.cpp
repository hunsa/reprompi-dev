//
// Created by Max on 12/27/22.
//

#include "wilcoxon_mann_whitney.h"

double WilcoxonMannWhitney::get_z_value() {

  double n = sample_1.size();
  double m = sample_2.size();
  double size_squared = n * m;
  double size_sum = n + m;
  double sample_1_rank_sum = 0;
  double sample_2_rank_sum = 0;

  std::vector<rank_element> ranks;

  for (auto iter = sample_1.begin(); iter != sample_1.end(); ++iter) {
    ranks.push_back({*iter,1});
  }

  for (auto iter = sample_2.begin(); iter != sample_2.end(); ++iter) {
    ranks.push_back({*iter,2});
  }

  std::vector<double> bounded_ranks;
  StatisticsUtils<double> statisticsUtils;
  std::sort(ranks.begin(), ranks.end());

  // add first rank if not bound
  if (ranks[1].runtime != ranks[2].runtime) {
    if (ranks[1].sample_id == 1) {
      sample_1_rank_sum += 1;
    } else {
      sample_2_rank_sum += 1;
    }
  }

  for (size_t rank = 2; rank <= ranks.size(); rank++) {
    // two values share the same rank
    if (ranks[rank - 1].runtime == ranks[rank].runtime) {
      if (bounded_ranks.empty()) {
        bounded_ranks.push_back(rank - 1);
      }
      bounded_ranks.push_back(rank);
    }
      // value is different from previous
    else {
      // but previous values shared the same rank
      if (!bounded_ranks.empty()) {
        double mean_bounded_ranks = statisticsUtils.mean(bounded_ranks);
        for (auto iter = bounded_ranks.begin(); iter != bounded_ranks.end(); ++iter) {
          if (ranks[*iter].sample_id == 1) {
            sample_1_rank_sum += mean_bounded_ranks;
          } else {
            sample_2_rank_sum += mean_bounded_ranks;
          }
        }
        bounded_ranks.clear();
      }
    }

    if (ranks[rank - 1].runtime != ranks[rank].runtime
        && ranks[rank + 1].runtime != ranks[rank].runtime) {
      if (ranks[rank].sample_id == 1) {
        sample_1_rank_sum += rank;
      } else {
        sample_2_rank_sum += rank;
      }
    }
  }

  if (!bounded_ranks.empty()) {
    double mean_bounded_ranks = statisticsUtils.mean(bounded_ranks);
    for (auto iter = bounded_ranks.begin(); iter != bounded_ranks.end(); ++iter) {
      if (ranks[*iter].sample_id == 1) {
        sample_1_rank_sum += mean_bounded_ranks;
      } else {
        sample_2_rank_sum += mean_bounded_ranks;
      }
    }
    bounded_ranks.clear();
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