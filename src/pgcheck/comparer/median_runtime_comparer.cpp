//
// Created by Max on 12/07/22.
//

#include "median_runtime_comparer.h"

static std::vector<int> col_widths = {30};

MedianRuntimeComparer::MedianRuntimeComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable MedianRuntimeComparer::get_results() {
  std::vector <std::string> col_names = {"message_size"};
  PGDataTable res(mpi_coll_name, col_names);
  StatisticsUtils<double> statisticsUtils;
  bool first = true;

  for (auto &count: mockup2data.at("default")->get_unique_counts()) {
    std::unordered_map <std::string, std::string> row;
    row["message_size"] = std::to_string(count);

    for (auto &mdata: mockup2data) {
      auto &data = mockup2data.at(mdata.first);
      auto rts = data->get_runtimes_for_count(count);

      if (first) {
        col_names.push_back(data->get_mockup_name());
        // 2 spaces padding on each side
        col_widths.push_back(data->get_mockup_name().size() + 4);
      }

      std::cout << "mockup:" << data->get_mockup_name() << std::endl;
      row[data->get_mockup_name()] = std::to_string(statisticsUtils.median(rts) * 1000);

    }
    std::cout << "row size:" << row.size() << std::endl;
    res.add_row(row);
    first = false;
  }

  std::cout << "size:" << col_names.size() << std::endl;
  res.set_col_names(col_names);
  res.set_col_widths(col_widths);
  return res;
}