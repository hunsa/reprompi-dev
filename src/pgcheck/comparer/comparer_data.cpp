//
// Created by Max on 10/30/22.
//

#include "comparer_data.h"

static const double critical_t_values[] = {0, 6.314, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,
                                           1.859548, 1.833113, 1.812461, 1.795885, 1.782288, 1.770933, 1.761310,
                                           1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718};
static const double normal_distribution_value = 1.644854;

ComparerData::ComparerData(int size, double mean, double median, double variance) :
  size(size),
  mean(mean),
  median(median),
  variance(variance),
  violation(false),
  fastest_mockup(""),
  fastest_mockup_median(0)
{}

double ComparerData::get_t_test(ComparerData alt_res) {
  int alt_df = alt_res.get_size() - 1;
  int def_df = size - 1;
  double standard_error = sqrt((alt_df * alt_res.get_variance() + def_df * variance) /
                               (alt_df + def_df));
  return sqrt((alt_res.get_size() * size) / (alt_res.get_size() + size)) * ((alt_res.get_mean() - mean) / standard_error);
}

bool ComparerData::get_violation(ComparerData alt_res) {
  double t_test = get_t_test(alt_res);
  int df = size + alt_res.get_size() - 2;
  double critical = normal_distribution_value;
  bool current_violation = t_test < -critical;
  if (df <= 20 && df >= 1) {
    critical = critical_t_values[df];
    current_violation = t_test < -critical;
  }
  return current_violation;
}

int ComparerData::get_size() {
  return size;
}

double ComparerData::get_mean() {
  return mean;
}

double ComparerData::get_median() {
  return median;
}

double ComparerData::get_median_ms() {
  return median * 1000;
}

double ComparerData::get_variance() {
  return variance;
}

double ComparerData::get_slowdown() {
  return median / fastest_mockup_median;
}

double ComparerData::get_fastest_mockup_median() {
  return fastest_mockup_median;
}

double ComparerData::get_fastest_mockup_median_ms() {
  return fastest_mockup_median * 1000;
}

std::string ComparerData::get_fastest_mockup() {
  return fastest_mockup;
}

bool ComparerData::is_violated() {
  return violation;
}

void ComparerData::set_fastest_mockup(std::string mockup, double mockup_median) {
  if(fastest_mockup_median == 0 || mockup_median < fastest_mockup_median) {
    violation = true;
    fastest_mockup = mockup;
    fastest_mockup_median = mockup_median;
  }
}










