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

#include "simple_comparer.h"

static std::vector<int> col_widths = {50, 10, 10};

SimpleComparer::SimpleComparer(const std::string& mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable SimpleComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "count", "runtime"};
  PGDataTable res(mpi_coll_name, col_names);
  StatisticsUtils<double> statisticsUtils;
  for (auto &mockup_data : mockup2data) {
    auto &data = mockup2data.at(mockup_data.first);
    for (auto &count : data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      std::unordered_map <std::string, std::string> row;
      row["mockup"] = mockup_data.first;
      row["count"] = std::to_string(count);
      row["runtime"] = std::to_string(statisticsUtils.mean(rts) * 1000);
      res.add_row(row);
    }
  }
  res.set_col_widths(col_widths);
  return res;
}
