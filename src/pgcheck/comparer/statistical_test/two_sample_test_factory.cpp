//
// Created by Max on 12/21/22.
//

#include "two_sample_test_factory.h"

TwoSampleTest *TwoSampleTestFactory::create_two_sample_test(int test_id) {
  TwoSampleTest *two_sample_test;
  switch (test_id) {
    default:
    case 0:
      two_sample_test = new TTest();
      break;
    case 1:
      two_sample_test = new WilcoxonRankSum();
      break;
    case 2:
      two_sample_test = new WilcoxonMannWhitney();
      break;
  }
  return two_sample_test;
}