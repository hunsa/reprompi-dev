//
// Created by Max on 10/30/22.
//

#include "wilcoxon.h"

static const double critical_t_values[] = {0, 6.313752, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,
                                           1.859548, 1.833113, 1.812461, 1.795885, 1.782288, 1.770933, 1.761310,
                                           1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718, 1.720743,
                                           1.717144, 1.713872, 1.710882};
static const double normal_distribution_value = 1.644854;

double Wilcoxon::get_z_value() {

  std::vector <std::pair<double, bool>> ranked_diff = get_ordered_diff_sign_vector();

  int rank_count = ranked_diff.size() - 1;
  double rank_sum_signed = 0;
  double rank_sum_unsigned = 0;
  double shared_rank_deviation = 0;
  StatisticsUtils<double> statisticsUtils;
  std::vector<double> bounded_ranks;
  bounded_ranks.clear();

  // add first rank if not bound
  if (ranked_diff[1].first != ranked_diff[2].first) {
    if (ranked_diff[1].second) {
      rank_sum_signed += 1;
    } else {
      rank_sum_unsigned += 1;
    }
  }

  for (int rank = 2; rank <= rank_count; rank++) {
    // two values share the same rank
    if (ranked_diff[rank - 1].first == ranked_diff[rank].first) {
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
          if (ranked_diff[*iter].second) {
            rank_sum_signed += mean_bounded_ranks;
          } else {
            rank_sum_unsigned += mean_bounded_ranks;
          }
        }

        shared_rank_deviation += get_shared_rank_deviation(bounded_ranks.size());
        bounded_ranks.clear();
      }
    }

    if (ranked_diff[rank - 1].first != ranked_diff[rank].first
        && ranked_diff[rank + 1].first != ranked_diff[rank].first) {
      if (ranked_diff[rank].second) {
        rank_sum_signed += rank;
      } else {
        rank_sum_unsigned += rank;
      }
    }

  }

  if (!bounded_ranks.empty()) {
    double mean_bounded_ranks = statisticsUtils.mean(bounded_ranks);
    for (auto iter = bounded_ranks.begin(); iter != bounded_ranks.end(); ++iter) {
      if (ranked_diff[*iter].second) {
        rank_sum_signed += mean_bounded_ranks;
      } else {
        rank_sum_unsigned += mean_bounded_ranks;
      }
    }

    shared_rank_deviation += get_shared_rank_deviation(bounded_ranks.size());
    bounded_ranks.clear();
  }

  double wilcoxon_value = std::min(rank_sum_signed, rank_sum_unsigned);
  double standard_deviation =
      std::sqrt((rank_count * (rank_count + 1) * (2 * rank_count + 1) - shared_rank_deviation) / 24);
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
  int size = std::min(sample_1.size(), sample_2.size());
  return (size * (size + 1)) / 4;
}

bool Wilcoxon::get_violation() {
  if (z_value != 0) {
    return z_value > get_critical_value();
  } else {
    return get_z_value() > get_critical_value();
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