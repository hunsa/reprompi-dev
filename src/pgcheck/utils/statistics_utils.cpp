//
// Created by Max on 10/30/22.
//

#include "statistics_utils.h"

template <class T>
StatisticsUtils<T>::StatisticsUtils()
{
}

template<class T>
T StatisticsUtils<T>::mean(std::vector <T> v) {
  T sum = std::accumulate(v.begin(), v.end(), 0.0);
  return sum / v.size();
}

template<class T>
T StatisticsUtils<T>::median(std::vector <T> v) {
  sort(v.begin(), v.end());
  if (v.size() % 2 == 0) {
    return (v[v.size() / 2] + v[v.size() / 2 - 1]) / 2;
  } else {
    return v[v.size() / 2];
  }
}

template<class T>
T StatisticsUtils<T>::variance(std::vector <T> v) {
  const size_t v_size = v.size();
  const T v_mean = mean(v);

  auto variance_function = [&v_mean, &v_size](T accumulator, const T &value) {
    return accumulator + ((value - v_mean) * (value - v_mean) / (v_size - 1));
  };

  return std::accumulate(v.begin(), v.end(), 0.0, variance_function);
}