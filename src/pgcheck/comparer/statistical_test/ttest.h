//
// Created by Max on 12/26/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TTEST_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TTEST_H

#include "two_sample_test.h"

class TTest : public TwoSampleTest {

public:

  TTest() = default;

  /**
   * @return critical t-value for degrees of freedom
   */
  double get_critical_value();

  /**
   * @return t-test-value for samples
   */
  double get_z_value();

  /**
   * @return true if t-test-value is smaller than critical t-value
   */
  bool get_violation();

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TTEST_H
