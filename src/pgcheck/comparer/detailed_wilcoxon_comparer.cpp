//
// Created by Max on 12/17/22.
//

#include "detailed_wilcoxon_comparer.h"

static std::vector<int> col_widths = {50, 15, 5, 5, 10, 15, 15, 15, 15, 15, 15, 10, 10, 13, 13};

DetailedWilcoxonComparer::DetailedWilcoxonComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn) {}

PGDataTable DetailedWilcoxonComparer::get_results() {
  std::vector <std::string> col_names = {"mockup", "count", "N", "ppn", "n", "default_mean", "default_median",
                                         "mockup_mean", "mockup_median",
                                         "wilcoxon_value", "critical_value", "violation", "slowdown", "has_barrier",
                                         "diff<barrier"};
  PGDataTable res(mpi_coll_name, col_names);
  std::unordered_map<int, TTest> def_res;
  std::unordered_map<int, Wilcoxon> wilcoxon_res;
  StatisticsUtils<double> statisticsUtils;
  auto &default_data = mockup2data.at("default");
  for (auto &count: default_data->get_unique_counts()) {
    auto rts_default = default_data->get_runtimes_for_count(count);
    auto *wilcoxon_data = new Wilcoxon(rts_default);
    TTest default_values(rts_default.size(), statisticsUtils.mean(rts_default),
                                statisticsUtils.median(rts_default), statisticsUtils.variance(rts_default));
    def_res.insert(std::make_pair(count, default_values));
    wilcoxon_res.insert(std::make_pair(count, *wilcoxon_data));
  }

  for (auto &mdata: mockup2data) {
    if (mdata.first == "default") {
      continue;
    }
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      wilcoxon_res.at(count).set_sample_2(rts);
      TTest alt_res(rts.size(), statisticsUtils.mean(rts),
                           statisticsUtils.median(rts), statisticsUtils.variance(rts));
      std::unordered_map <std::string, std::string> row;
      row["mockup"] = mdata.first;
      row["count"] = std::to_string(count);
      row["N"] = std::to_string(nnodes);
      row["ppn"] = std::to_string(ppn);
      row["n"] = std::to_string(alt_res.get_size());
      row["default_mean"] = std::to_string(def_res.at(count).get_mean_ms());
      row["default_median"] = std::to_string(def_res.at(count).get_median_ms());
      row["mockup_mean"] = std::to_string(alt_res.get_mean_ms());
      row["mockup_median"] = std::to_string(alt_res.get_median_ms());
      row["wilcoxon_value"] = std::to_string(wilcoxon_res.at(count).get_z_value());
      row["critical_value"] = std::to_string(wilcoxon_res.at(count).get_critical_value());
      row["violation"] = std::to_string(wilcoxon_res.at(count).get_violation());
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