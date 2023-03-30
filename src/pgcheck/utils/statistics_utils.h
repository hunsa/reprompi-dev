/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#ifndef SRC_PGCHECK_UTILS_STATISTICS_UTILS_H_
#define SRC_PGCHECK_UTILS_STATISTICS_UTILS_H_

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

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

#endif  // SRC_PGCHECK_UTILS_STATISTICS_UTILS_H_
