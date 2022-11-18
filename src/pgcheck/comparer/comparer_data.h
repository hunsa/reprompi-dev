//
// Created by Max on 10/30/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H

#include <string>
#include <vector>
#include <numeric>
#include <cmath>

class ComparerData {

private:
  bool violation;
  int size;
  double mean;
  double median;
  double variance;
  double fastest_mockup_median;
  std::string fastest_mockup;

  double get_mean();

  double get_variance();


public:
  ComparerData(int size, double mean, double median, double variance);

  bool is_violated();

  /**
   * @return true if t-value of data is smaller than critical t-value
   */
  bool get_violation(ComparerData data);

  int get_size();

  double get_mean_ms();

  double get_median();

  double get_median_ms();

  /**
   * @return critical t-value for sum of dfs
   */
  double get_critical_t_value(int df);

  double get_fastest_mockup_median_ms();

  double get_slowdown();

  /**
   * @return slowdown between this and mockup_median
   */
  double get_slowdown(double mockup_median);

  /**
   * @return t-value for this and data sample
   */
  double get_t_test(ComparerData data);

  std::string get_fastest_mockup();

  /**
   * overwrites fastest mockup is mockup_median is smaller than current
   */
  void set_fastest_mockup(std::string mockup, double mockup_median);

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H
