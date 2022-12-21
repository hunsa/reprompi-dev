//
// Created by Max on 12/18/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TEST_WILCOXON_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TEST_WILCOXON_H

#include "two_sample_test.h"
#include <assert.h>

class Wilcoxon : public TwoSampleTest {

private:

  std::vector <std::pair<double, bool>> get_ordered_diff_sign_vector();

  double get_shared_rank_deviation(int shared_rank_count);

  double get_expected_value();


public:

  Wilcoxon() = default;

  /**
   * @return critical wilcoxon value for min sample size
   */
  double get_critical_value();

  /**
   * @return wilcoxon-sum-rank-test value for samples
   */
  double get_z_value();

  /**
   * @return true if wilcoxon-test value is smaller than critical wilcoxon value
   */
  bool get_violation();

};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TEST_WILCOXON_H
