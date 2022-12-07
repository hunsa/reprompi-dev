//
// Created by Max on 12/07/22.
//

#include "rel_runtime_comparer.h"

RelRuntimeComparer::RelRuntimeComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable RelRuntimeComparer::get_results() {
  std::vector <std::string> col_names = {"message_size", "mockup", "median_ms"};
  std::vector<int> col_widths = {15, 50, 15};
  PGDataTable res(mpi_coll_name, col_names);
  StatisticsUtils<double> statisticsUtils;

  auto &default_data = mockup2data.at("default");
  std::unordered_map<int, double> def_res;
  for (auto &count: default_data->get_unique_counts()) {
    auto rts_default = default_data->get_runtimes_for_count(count);
    def_res.insert(std::make_pair(count, statisticsUtils.median(rts_default)));
    std::unordered_map <std::string, std::string> row;
    row["message_size"] = std::to_string(count);
    row["mockup"] = "default";
    row["median_ms"] = std::to_string(1);
    res.add_row(row);
  }

  for (auto &mdata: mockup2data) {
    if (mdata.first == "default") {
      continue;
    }
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      std::unordered_map <std::string, std::string> row;
      row["message_size"] = std::to_string(count);
      row["mockup"] = mdata.first;
      row["median_ms"] = std::to_string(def_res.at(count) / statisticsUtils.median(rts));
      res.add_row(row);
    }
  }

  res.set_col_widths(col_widths);
  return res;
}