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

#include "detailed_violation_comparer.h"

static std::vector<int> col_widths = {50, 15, 5, 5, 10, 15, 15, 15, 15, 15, 15, 10, 10, 13, 13};

DetailedViolationComparer::DetailedViolationComparer(int test_type,
                                                     const std::string &mpi_coll_name,
                                                     int nnodes,
                                                     int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn), test_type(test_type) {}

PGDataTable DetailedViolationComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "count", "N", "ppn", "n", "default_mean", "default_median",
                                         "mockup_mean", "mockup_median",
                                         "z_value", "critical_val", "violation", "slowdown", "has_barrier",
                                         "diff<barrier"};
  PGDataTable res(mpi_coll_name, col_names);
  std::unordered_map<int, ComparerData> def_res;
  auto &default_data = mockup2data.at("default");
  for (auto &count : default_data->get_unique_counts()) {
    auto rts_default = default_data->get_runtimes_for_count(count);
    ComparerData default_values(rts_default, test_type);
    def_res.insert(std::make_pair(count, default_values));
  }

  for (auto &mockup_data : mockup2data) {
    if (mockup_data.first == "default") {
      continue;
    }
    auto &data = mockup2data.at(mockup_data.first);
    for (auto &count : data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      ComparerData alt_res(rts);
      std::unordered_map <std::string, std::string> row;
      row["mockup"] = mockup_data.first;
      row["count"] = std::to_string(count);
      row["N"] = std::to_string(nnodes);
      row["ppn"] = std::to_string(ppn);
      row["n"] = std::to_string(alt_res.get_size());
      row["default_mean"] = std::to_string(def_res.at(count).get_mean_ms());
      row["default_median"] = std::to_string(def_res.at(count).get_median_ms());
      row["mockup_mean"] = std::to_string(alt_res.get_mean_ms());
      row["mockup_median"] = std::to_string(alt_res.get_median_ms());
      row["z_value"] = std::to_string(def_res.at(count).get_z_value(alt_res));
      row["critical_val"] = std::to_string(def_res.at(count).get_critical_value(alt_res));
      row["violation"] = std::to_string(def_res.at(count).get_violation(alt_res));
      row["slowdown"] = std::to_string(def_res.at(count).get_slowdown(alt_res.get_median()));
      if (has_barrier_time()) {
        row["has_barrier"] = "*";
        if (def_res.at(count).get_median_ms() - alt_res.get_median_ms() < get_barrier_time() * 1000) {
          row["diff<barrier"] = "*";
        } else {
          row["diff<barrier"] = " ";
        }
      } else {
        row["has_barrier"] = " ";
      }
      res.add_row(row);
    }
  }
  res.set_col_widths(col_widths);
  return res;
}
