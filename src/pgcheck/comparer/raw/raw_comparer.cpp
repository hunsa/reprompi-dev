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

#include "raw_comparer.h"

RawComparer::RawComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable RawComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "message_size", "run_id", "runtime"};
  std::vector<int> col_widths = {50, 15, 10, 10};
  PGDataTable res(mpi_coll_name, col_names);

  for (auto &mdata : mockup2data) {
    auto &data = mockup2data.at(mdata.first);
    for (auto &count : data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      size_t run_id = 0;
      for (auto current_rts : rts) {
        std::unordered_map <std::string, std::string> row;
        row["mockup"] = mdata.first;
        row["message_size"] = std::to_string(count);
        row["run_id"] = std::to_string(run_id++);
        row["runtime"] = std::to_string(current_rts);
        res.add_row(row);
      }
    }
  }

  res.set_col_widths(col_widths);
  return res;
}
