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

public:
  ComparerData(int size, double mean, double median, double variance);
  int get_size();
  double get_mean_ms();
  double get_median();
  double get_median_ms();
  bool is_violated();
  bool get_violation(ComparerData data);
  std::string get_fastest_mockup();
  void set_fastest_mockup(std::string mockup, double mockup_median);
  double get_critical_t_value(int alt_size);
  double get_fastest_mockup_median_ms();
  double get_slowdown();
  double get_slowdown(double mockup_median);
  double get_t_test(ComparerData data);

private:
  int size;
  double mean;
  double median;
  double variance;
  bool violation;
  std::string fastest_mockup;
  double fastest_mockup_median;
  double get_mean();
  double get_variance();
};



#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_DATA_H
