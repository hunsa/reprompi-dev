//
// Created by Max on 10/30/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H

#include "statistical_test/two_sample_test.h"
#include "statistical_test/two_sample_test_factory.h"
#include <string>
#include <vector>
#include <numeric>
#include <cmath>

class ComparerData {

private:

  bool violation; // indicates if this object is affected by a guideline violation
  double fastest_mockup_median; // median of the fastest mockup which violates the guidelines
  std::string fastest_mockup; // name of the fastest mockup
  std::vector<double> runtimes; // a vector of runtimes
  TwoSampleTest *two_sample_test; // the test object

  double get_mean();

  double get_variance();


public:

  ComparerData(std::vector<double> rts);

  ComparerData(std::vector<double> rts, int test_id);

  bool is_violated();

  /**
   * @return true if samples violate test hypothesis
   */
  bool get_violation(ComparerData alt);

  int get_size();

  double get_mean_ms();

  double get_median();

  double get_median_ms();

  /**
   * @return critical value based on chosen test type
   */
  double get_critical_value(ComparerData alt);

  double get_fastest_mockup_median_ms();

  double get_slowdown();

  /**
   * @return slowdown between this and mockup_median
   */
  double get_slowdown(double mockup_median);

  /**
   * @return z-value for this and alt data based on chosen test type
   */
  double get_z_value(ComparerData alt);

  std::string get_fastest_mockup();

  /**
   * overwrites fastest mockup if mockup_median is smaller than current
   */
  void set_fastest_mockup(std::string mockup, double mockup_median);

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H
