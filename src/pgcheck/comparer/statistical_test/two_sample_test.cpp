//
// Created by Max on 10/30/22.
//

#include "two_sample_test.h"

static const double critical_values[] = {0, 6.313752, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,
                                           1.859548, 1.833113, 1.812461, 1.795885, 1.782288, 1.770933, 1.761310,
                                           1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718, 1.720743,
                                           1.717144, 1.713872, 1.710882};
static const double normal_distribution_value = 1.644854;

TwoSampleTest::TwoSampleTest(std::vector<double> sample) :
    sample_1(sample)
{}

double TwoSampleTest::get_critical_value(int sample_size) {
  if (sample_size < 25) {
    return critical_values[sample_size];
  } else {
    return normal_distribution_value;
  }
}