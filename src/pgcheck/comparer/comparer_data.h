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

#ifndef SRC_PGCHECK_COMPARER_COMPARER_DATA_H_
#define SRC_PGCHECK_COMPARER_COMPARER_DATA_H_

#include <string>
#include <vector>
#include <numeric>
#include <cmath>

#include "statistical_test/two_sample_test.h"
#include "statistical_test/two_sample_test_factory.h"

class ComparerData {
 private:
  bool violation = false;            // indicates if this object is affected by a guideline violation
  double fastest_mockup_median = 0;  // median of the fastest mockup which violates the guidelines
  std::string fastest_mockup;        // name of the fastest mockup
  std::vector<double> runtimes;      // a vector of runtimes
  TwoSampleTest *two_sample_test;    // the test object
  double get_mean();
  double get_variance();

 public:
  explicit ComparerData(std::vector<double> rts);
  ComparerData(std::vector<double> rts, int test_id);
  bool is_violated();
  /**
   * @return true if samples violate test hypothesis
   */
  bool get_violation(ComparerData alt);
  int get_size();
  double get_mean_ms();
  double get_median();
  double get_median_ms();
  /**
   * @return critical value based on chosen test type
   */
  double get_critical_value(ComparerData alt);
  double get_fastest_mockup_median_ms();
  double get_slowdown();
  /**
   * @return slowdown between this and mockup_median
   */
  double get_slowdown(double mockup_median);
  /**
   * @return z-value for this and alt data based on chosen test type
   */
  double get_z_value(ComparerData alt);
  std::string get_fastest_mockup();
  /**
   * overwrites fastest mockup if mockup_median is smaller than current
   */
  void set_fastest_mockup(std::string mockup, double mockup_median);
};

#endif  // SRC_PGCHECK_COMPARER_COMPARER_DATA_H_
