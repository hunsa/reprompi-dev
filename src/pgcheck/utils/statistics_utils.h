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

  StatisticsUtils() = default;

  T mean(std::vector <T> v) {
    T sum = std::accumulate(v.begin(), v.end(), 0.0);
    return sum / v.size();
  }

  T median(std::vector <T> v) {
    sort(v.begin(), v.end());
    if (v.size() % 2 == 0) {
      return (v[v.size() / 2] + v[v.size() / 2 - 1]) / 2;
    } else {
      return v[v.size() / 2];
    }
  }

  T variance(std::vector <T> v) {
    const size_t v_size = v.size();
    const T v_mean = mean(v);

    auto variance_function = [&v_mean, &v_size](T accumulator, const T &value) {
      return accumulator + ((value - v_mean) * (value - v_mean) / (v_size - 1));
    };

    return std::accumulate(v.begin(), v.end(), 0.0, variance_function);
  }
};

#endif //REPROMPI_DEV_SRC_PGCHECK_UTILS_STATISTICS_UTILS_H
