//
// Created by Max on 12/18/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_WILCOXON_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_WILCOXON_H

#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <cmath>
#include <algorithm>
#include "../../utils/statistics_utils.h"

class Wilcoxon {

private:
  std::vector<double> sample_1;
  std::vector<double> sample_2;
  double z_value;

  std::vector<std::pair<double, bool>> get_ordered_diff_sign_vector();
  double get_shared_rank_deviation(int shared_rank_count);
  double get_expected_value();


public:
  Wilcoxon(std::vector<double> sample);

  void set_sample_1(std::vector<double> sample);

  void set_sample_2(std::vector<double> sample);

  double get_wilcoxon_value();

  double get_critical_value();

  bool get_violation();

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_WILCOXON_H
