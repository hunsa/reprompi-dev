//
// Created by Max on 12/21/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TWO_SAMPLE_TEST_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TWO_SAMPLE_TEST_H

#include <string>
#include <vector>
#include <numeric>
#include <cmath>

class TwoSampleTest {

protected:
  std::vector<double> sample_1;
  std::vector<double> sample_2;

public:

  TwoSampleTest(std::vector<double> sample);

  virtual ~TwoSampleTest() {};

  virtual double get_z_value() = 0;

  virtual bool get_violation() = 0;

  double get_critical_value(int sample_size);

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TESTS_TWO_SAMPLE_TEST_H
