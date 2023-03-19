//
// Created by Max on 03/19/23.
//

#include "raw_comparer.h"

RawComparer::RawComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable RawComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "message_size", "run_id", "runtime"};
  std::vector<int> col_widths = {50, 15, 10, 10};
  PGDataTable res(mpi_coll_name, col_names);

  for (auto &mdata: mockup2data) {
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      size_t run_id = 0;
      for(auto current_rts : rts) {
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