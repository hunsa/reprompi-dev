//
// Created by Sascha on 10/2/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H
#define REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H

#include <string>
#include <vector>
#include <cmath>
#include <numeric>

template<class T>
class StatisticsUtils {

public:
  StatisticsUtils();
  T mean(std::vector <T> v);
  T median(std::vector <T> v);
  T variance(std::vector <T> v);
};

#endif //REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H
