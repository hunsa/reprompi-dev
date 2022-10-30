//
// Created by Max on 10/30/22.
//

#include "default_comparer.h"

static const int col_widths[] = {50, 10, 10};

DefaultResults::DefaultResults(std::string mpi_name, std::vector <std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names) {
}

std::string DefaultResults::get() {
  std::stringstream res;
  res << "MPI Collective: " << this->mpi_name << "\n";
  int idx = 0;
  for (auto &colname: this->col_names) {
    res << std::setw(col_widths[idx++]) << colname << " ";
  }
  res << "\n";
  int nb_rows = this->col_value_map.at(this->col_names[0]).size();
  for (int i = 0; i < nb_rows; i++) {
    idx = 0;
    for (auto &colname: this->col_names) {
      auto &values = this->col_value_map.at(colname);
      res << std::setw(col_widths[idx++]) << values[i] << " ";
    }
    res << "\n";
  }
  return res.str();
}

void DefaultResults::add_row(std::unordered_map <std::string, std::string> &row_map) {
  for (auto &rows: row_map) {
    col_value_map[rows.first].push_back(rows.second);
  }
}


DefaultComparer::DefaultComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

std::string DefaultComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "count", "runtime"};
  DefaultResults res(mpi_coll_name, col_names);
  StatisticsUtils<double> statisticsUtils;
  std::cout << "nnodes: " << nnodes << " ppn: " << ppn << std::endl;
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
  return res.get();
}