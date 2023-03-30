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

#ifndef SRC_PGCHECK_COMPARER_STATISTICAL_TEST_TWO_SAMPLE_TEST_H_
#define SRC_PGCHECK_COMPARER_STATISTICAL_TEST_TWO_SAMPLE_TEST_H_

#include <assert.h>

#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <algorithm>


#include "../../utils/statistics_utils.h"

class TwoSampleTest {
 protected:
  std::vector<double> sample_1;
  std::vector<double> sample_2;
  double z_value;  // value is set by subclass implementation if test was successful

 public:
  TwoSampleTest() = default;
  virtual ~TwoSampleTest() {}
  /**
   * @return critical value based on concrete test subclass
   */
  virtual double get_critical_value() = 0;
  /**
   * @return z-test-value based on concrete test subclass
   */
  virtual double get_z_value() = 0;
  virtual bool get_violation() = 0;
  /**
   * @return critical value at index; normal distribution value if sample size >= 25
   */
  double get_critical_value(int sample_size);
  void set_samples(std::vector<double> def, std::vector<double> alt);
};

#endif  // SRC_PGCHECK_COMPARER_STATISTICAL_TEST_TWO_SAMPLE_TEST_H_
