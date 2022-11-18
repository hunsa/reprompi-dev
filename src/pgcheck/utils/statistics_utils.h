//
// Created by Sascha on 10/2/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H
#define REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H

#include <vector>
#include <cmath>
#include <numeric>

template<class T>
class StatisticsUtils {

public:
  StatisticsUtils();
  /**
   * @return mean of a vector in any numerical format
   */
  T mean(std::vector <T> v);
  /**
   * @return median of a vector in any numerical format
   */
  T median(std::vector <T> v);
  /**
   * @return variance of a vector in any numerical format
   */
  T variance(std::vector <T> v);
};

#endif //REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H
