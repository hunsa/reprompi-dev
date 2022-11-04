//
// Created by Sascha on 10/22/22.
//

#include "ttest_comparer.h"

static const int col_widths[] = {50, 15, 5, 5, 10, 15, 15, 15, 15, 5};

TTestResults::TTestResults(std::string mpi_name, std::vector <std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names) {
}

std::string TTestResults::get() {
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

void TTestResults::add_row(std::unordered_map <std::string, std::string> &row_map) {
  for (auto &rows: row_map) {
    col_value_map[rows.first].push_back(rows.second);
  }
}

TTestComparer::TTestComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

std::string TTestComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "count", "N", "ppn", "n", "runtime_mean", "runtime_median",
                                         "t_value", "critical_t_value", "violation"};
  TTestResults res(mpi_coll_name, col_names);
  std::unordered_map<int, ComparerData> def_res;
  StatisticsUtils<double> statisticsUtils;
  auto &default_data = mockup2data.at("default");
  for (auto &count: default_data->get_unique_counts()) {
    auto rts_default = default_data->get_runtimes_for_count(count);
    ComparerData default_values(rts_default.size(), statisticsUtils.mean(rts_default),
                                statisticsUtils.median(rts_default), statisticsUtils.variance(rts_default));
    std::unordered_map <std::string, std::string> row;
    row["mockup"] = "default";
    row["count"] = std::to_string(count);
    row["N"] = std::to_string(nnodes);
    row["ppn"] = std::to_string(ppn);
    row["n"] = std::to_string(default_values.get_size());
    row["runtime_mean"] = std::to_string(default_values.get_mean_ms());
    row["runtime_median"] = std::to_string(default_values.get_median_ms());
    row["critical_t_value"] = "";
    row["t_value"] = "";
    row["violation"] = "";
    def_res.insert(std::make_pair(count, default_values));
    res.add_row(row);
  }

  for (auto &mdata: mockup2data) {
    if (mdata.first == "default") {
      continue;
    }
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      ComparerData alt_res(rts.size(), statisticsUtils.mean(rts),
                           statisticsUtils.median(rts), statisticsUtils.variance(rts));
      std::unordered_map <std::string, std::string> row;
      row["mockup"] = mdata.first;
      if( has_barrier_time() ) {
        // don't forget, barrier time is in 's'
        if( def_res.at(count).get_median_ms() - alt_res.get_median_ms() < get_barrier_time() * 1000 ) {
          row["mockup"] = row["mockup"] + "*";
        }
      }
      row["count"] = std::to_string(count);
      row["N"] = std::to_string(nnodes);
      row["ppn"] = std::to_string(ppn);
      row["n"] = std::to_string(alt_res.get_size());
      row["runtime_mean"] = std::to_string(alt_res.get_mean_ms());
      row["runtime_median"] = std::to_string(alt_res.get_median_ms());
      row["t_value"] = std::to_string(def_res.at(count).get_t_test(alt_res));
      row["critical_t_value"] = std::to_string(def_res.at(count).get_critical_t_value(alt_res.get_size()));
      row["violation"] = std::to_string(def_res.at(count).get_violation(alt_res));
      res.add_row(row);
    }
  }
  return res.get();
}