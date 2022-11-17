//
// Created by Max on 10/30/22.
//

#include "simple_comparer.h"

static std::vector<int> col_widths = {50, 10, 10};

SimpleComparer::SimpleComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable SimpleComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "count", "runtime"};
  PGDataTable res(mpi_coll_name, col_names);
  StatisticsUtils<double> statisticsUtils;
  for (auto &mdata: mockup2data) {
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      std::unordered_map <std::string, std::string> row;
      row["mockup"] = mdata.first;
      row["count"] = std::to_string(count);
      row["runtime"] = std::to_string(statisticsUtils.mean(rts)*1000);
      res.add_row(row);
    }
  }
  res.set_col_widths(col_widths);
  return res;
}