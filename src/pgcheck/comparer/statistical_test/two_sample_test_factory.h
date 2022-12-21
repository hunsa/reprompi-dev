//
// Created by Max on 12/21/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TEST_TWO_SAMPLE_TEST_FACTORY_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TEST_TWO_SAMPLE_TEST_FACTORY_H

#include "two_sample_test.h"
#include "ttest.h"
#include "wilcoxon.h"

class TwoSampleTestFactory {
public:
  static TwoSampleTest* create_two_sample_test(int test_id);
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_STATISTICAL_TEST_TWO_SAMPLE_TEST_FACTORY_H
