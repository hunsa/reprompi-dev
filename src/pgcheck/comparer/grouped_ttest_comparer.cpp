//
// Created by Max on 10/30/22.
//

#include "grouped_ttest_comparer.h"

static const int col_widths[] = {15, 15, 5, 5, 5, 15, 10, 50, 15};

GroupedTTestResults::GroupedTTestResults(std::string mpi_name, std::vector <std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names) {
}

std::string GroupedTTestResults::get() {
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

void GroupedTTestResults::add_row(std::unordered_map <std::string, std::string> &row_map) {
  for (auto &rows: row_map) {
    col_value_map[rows.first].push_back(rows.second);
  }
}

GroupedTTestComparer::GroupedTTestComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

std::string GroupedTTestComparer::get_results() {

  std::vector <std::string> col_names = {"collective", "count", "N", "ppn", "n", "default_median", "slowdown", "mockup",
                                         "mockup_median"};
  GroupedTTestResults res(mpi_coll_name, col_names);
  std::map<int, ComparerData> def_res;
  auto &default_data = mockup2data.at("default");
  StatisticsUtils<double> statisticsUtils;

  for (auto &count: default_data->get_unique_counts()) {
    auto rts_default = default_data->get_runtimes_for_count(count);
    ComparerData default_values(rts_default.size(), statisticsUtils.mean(rts_default), statisticsUtils.median(rts_default), statisticsUtils.variance(rts_default));
    def_res.insert(std::make_pair(count, default_values));
  }

  for (auto &mdata: mockup2data) {
    if (mdata.first == "default") {
      continue;
    }
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      ComparerData alt_res(rts.size(), statisticsUtils.mean(rts), statisticsUtils.median(rts), statisticsUtils.variance(rts));
      if (def_res.at(count).get_violation(alt_res)) {
        def_res.at(count).set_fastest_mockup(mdata.first, alt_res.get_median());
      }
    }
  }

  for (auto &count: def_res) {
    auto &data = def_res.at(count.first);
    std::unordered_map <std::string, std::string> row;
    row["collective"] = mpi_coll_name;
    row["count"] = std::to_string(count.first);
    row["N"] = std::to_string(nnodes);
    row["ppn"] = std::to_string(ppn);
    row["n"] = std::to_string(data.get_size());
    row["default_median"] = std::to_string(data.get_median_ms());
    if (data.is_violated()) {
      row["slowdown"] = std::to_string(data.get_slowdown());
      row["mockup"] = data.get_fastest_mockup();
      row["mockup_median"] = std::to_string(data.get_fastest_mockup_median_ms());
    } else {
      row["slowdown"] = "";
      row["mockup"] = "";
      row["mockup_median"] = "";
    }
    res.add_row(row);
  }
  return res.get();
}