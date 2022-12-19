//
// Created by Max on 10/30/22.
//

#include "ttest.h"

static const double critical_t_values[] = {0, 6.314, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,
                                           1.859548, 1.833113, 1.812461, 1.795885, 1.782288, 1.770933, 1.761310,
                                           1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718};
static const double normal_distribution_value = 1.644854;

TTest::TTest(int size, double mean, double median, double variance) :
    violation(false),
    size(size),
    mean(mean),
    median(median),
    variance(variance),
    fastest_mockup_median(0),
    fastest_mockup("") {}

bool TTest::is_violated() {
  return violation;
}

bool TTest::get_violation(TTest alt_res) {
  return get_t_test(alt_res) < -get_critical_t_value(alt_res.get_size());
}

int TTest::get_size() {
  return size;
}

double TTest::get_mean() {
  return mean;
}

double TTest::get_variance() {
  return variance;
}

double TTest::get_mean_ms() {
  return mean * 1000;
}

double TTest::get_median() {
  return median;
}

double TTest::get_median_ms() {
  return median * 1000;
}

double TTest::get_critical_t_value(int df) {
  int df_sum = size + df - 2;
  double critical = normal_distribution_value;
  if (df_sum <= 20 && df_sum >= 1) {
    critical = critical_t_values[df_sum];
  }
  return critical;
}

double TTest::get_fastest_mockup_median_ms() {
  return fastest_mockup_median * 1000;
}

double TTest::get_slowdown() {
  return median / fastest_mockup_median;
}

double TTest::get_slowdown(double mockup_median) {
  return median / mockup_median;
}

double TTest::get_t_test(TTest alt_res) {
  int alt_df = alt_res.get_size() - 1;
  int def_df = size - 1;
  double standard_error = sqrt((alt_df * alt_res.get_variance() + def_df * variance) /
                               (alt_df + def_df));
  return sqrt((alt_res.get_size() * size) / (alt_res.get_size() + size)) *
         ((alt_res.get_mean() - mean) / standard_error);
}

std::string TTest::get_fastest_mockup() {
  return fastest_mockup;
}

void TTest::set_fastest_mockup(std::string mockup, double mockup_median) {
  if (fastest_mockup_median == 0 || mockup_median < fastest_mockup_median) {
    violation = true;
    fastest_mockup = mockup;
    fastest_mockup_median = mockup_median;
  }
}