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

#include "ttest.h"

double TTest::get_critical_value() {
  int df_sum = sample_1.size() + sample_2.size() - 2;
  return TwoSampleTest::get_critical_value(df_sum);
}

double TTest::get_z_value() {
  int def_df = sample_1.size() - 1;
  int alt_df = sample_2.size() - 1;
  StatisticsUtils<double> statistics_utils;

  double standard_error = sqrt(
      (alt_df * statistics_utils.variance(sample_2) + def_df * statistics_utils.variance(sample_1)) /
      (alt_df + def_df));
  return sqrt((sample_1.size() * sample_2.size()) / (sample_1.size() + sample_2.size())) *
         ((statistics_utils.mean(sample_2) - statistics_utils.mean(sample_1)) / standard_error);
}

bool TTest::get_violation() {
  return get_z_value() < -get_critical_value();
}
