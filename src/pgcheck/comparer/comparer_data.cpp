//
// Created by Max on 10/30/22.
//

#include "comparer_data.h"

static const double critical_t_values[] = {0, 6.314, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,
                                           1.859548, 1.833113, 1.812461, 1.795885, 1.782288, 1.770933, 1.761310,
                                           1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718};
static const double normal_distribution_value = 1.644854;

ComparerData::ComparerData(std::vector<double> rts) : runtimes(rts) {}

ComparerData::ComparerData(std::vector<double> rts, int test_id) {
  runtimes = rts;
  two_sample_test = TwoSampleTestFactory::create_two_sample_test(test_id);
}

bool ComparerData::is_violated() {
  return violation;
}

int ComparerData::get_size() {
  return runtimes.size();
}

double ComparerData::get_mean() {
  StatisticsUtils<double> statisticsUtils;
  return statisticsUtils.mean(runtimes);
}

double ComparerData::get_variance() {
  StatisticsUtils<double> statisticsUtils;
  return statisticsUtils.variance(runtimes);
}

double ComparerData::get_mean_ms() {
  return get_mean() * 1000;
}

double ComparerData::get_median() {
  StatisticsUtils<double> statisticsUtils;
  return statisticsUtils.median(runtimes);
}

double ComparerData::get_median_ms() {
  return get_median() * 1000;
}

double ComparerData::get_fastest_mockup_median_ms() {
  return fastest_mockup_median * 1000;
}

double ComparerData::get_slowdown() {
  return get_median() / fastest_mockup_median;
}

double ComparerData::get_slowdown(double mockup_median) {
  return get_median() / mockup_median;
}

std::string ComparerData::get_fastest_mockup() {
  return fastest_mockup;
}

void ComparerData::set_fastest_mockup(std::string mockup, double mockup_median) {
  if (fastest_mockup_median == 0 || mockup_median < fastest_mockup_median) {
    violation = true;
    fastest_mockup = mockup;
    fastest_mockup_median = mockup_median;
  }
}

double ComparerData::get_z_value(ComparerData alt) {
  two_sample_test->set_samples(runtimes, alt.runtimes);
  return two_sample_test->get_z_value();
}

double ComparerData::get_critical_value(ComparerData alt) {
  two_sample_test->set_samples(runtimes, alt.runtimes);
  return two_sample_test->get_critical_value();
}

bool ComparerData::get_violation(ComparerData alt) {
  two_sample_test->set_samples(runtimes, alt.runtimes);
  return two_sample_test->get_violation();
}