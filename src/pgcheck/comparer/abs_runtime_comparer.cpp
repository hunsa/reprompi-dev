//
// Created by Max on 12/07/22.
//

#include "abs_runtime_comparer.h"

AbsRuntimeComparer::AbsRuntimeComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable AbsRuntimeComparer::get_results() {
  std::vector <std::string> col_names = {"message_size", "mockup", "median_ms"};
  std::vector<int> col_widths = {15, 50, 10};
  PGDataTable res(mpi_coll_name, col_names);
  StatisticsUtils<double> statisticsUtils;

  for (auto &mdata: mockup2data) {
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      std::unordered_map <std::string, std::string> row;
      row["message_size"] = std::to_string(count);
      row["mockup"] = mdata.first;
      row["median_ms"] = std::to_string(statisticsUtils.mean(rts) * 1000);
      res.add_row(row);
    }
  }

  res.set_col_widths(col_widths);
  return res;
}