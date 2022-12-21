//
// Created by Max on 12/26/22.
//

#include "ttest.h"

double TTest::get_critical_value() {
  int df_sum = sample_1.size() + sample_2.size() - 2;
  return TwoSampleTest::get_critical_value(df_sum);
}

double TTest::get_z_value() {
  int def_df = sample_1.size() - 1;
  int alt_df = sample_2.size() - 1;
  StatisticsUtils<double> statistics_utils;

  double standard_error = sqrt((alt_df * statistics_utils.variance(sample_2) + def_df * statistics_utils.variance(sample_1)) /
                               (alt_df + def_df));
  return sqrt((sample_1.size() * sample_2.size()) / (sample_1.size() + sample_2.size())) *
         ((statistics_utils.mean(sample_2) - statistics_utils.mean(sample_1)) / standard_error);
}

bool TTest::get_violation() {
  return get_z_value() < -get_critical_value();
}