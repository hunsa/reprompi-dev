//
// Created by Max on 12/21/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TWO_SAMPLE_TEST_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TWO_SAMPLE_TEST_H

#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <algorithm>
#include <assert.h>
#include "../../utils/statistics_utils.h"

class TwoSampleTest {

protected:
  std::vector<double> sample_1;
  std::vector<double> sample_2;
  double z_value; // value is set by subclass implementation if test was successful

public:

  TwoSampleTest() = default;

  virtual ~TwoSampleTest() {};

  /**
   * @return critical value based on concrete test subclass
   */
  virtual double get_critical_value() = 0;

  /**
   * @return z-test-value based on concrete test subclass
   */
  virtual double get_z_value() = 0;

  virtual bool get_violation() = 0;

  /**
   * @return critical value at index; normal distribution value if sample size >= 25
   */
  double get_critical_value(int sample_size);

  void set_samples(std::vector<double> def, std::vector<double> alt);

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TWO_SAMPLE_TEST_H
