//
// Created by Max on 10/30/22.
//

#include "grouped_violation_comparer.h"

static std::vector<int> col_widths = {25, 15, 5, 5, 5, 15, 10, 50, 15};

GroupedViolationComparer::GroupedViolationComparer(int test_type, std::string mpi_coll_name, int nnodes, int ppn) :
    PGDataComparer(mpi_coll_name, nnodes, ppn), test_type(test_type) {}

PGDataTable GroupedViolationComparer::get_results() {

  std::vector <std::string> col_names = {"collective", "count", "N", "ppn", "n", "default_median", "slowdown", "mockup",
                                         "mockup_median"};
  PGDataTable res(mpi_coll_name, col_names);
  std::map<int, ComparerData> def_res;
  auto &default_data = mockup2data.at("default");

  for (auto &count: default_data->get_unique_counts()) {
    auto rts_default = default_data->get_runtimes_for_count(count);
    ComparerData default_values(rts_default, test_type);
    def_res.insert(std::make_pair(count, default_values));
  }

  for (auto &mdata: mockup2data) {
    if (mdata.first == "default") {
      continue;
    }
    auto &data = mockup2data.at(mdata.first);
    for (auto &count: data->get_unique_counts()) {
      auto rts = data->get_runtimes_for_count(count);
      ComparerData alt_res(rts);
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

    std::cout << std::to_string(data.get_slowdown()) << std::endl;
    std::cout << std::to_string(data.get_median()) << std::endl;
    std::cout << std::endl;

    /*if (data.is_violated()) {
      row["slowdown"] = std::to_string(data.get_slowdown());
      row["mockup"] = data.get_fastest_mockup();
      row["mockup_median"] = std::to_string(data.get_fastest_mockup_median_ms());
      if( has_barrier_time() ) {
        // don't forget, barrier time is in 's'
        if( data.get_median_ms() - data.get_fastest_mockup_median_ms() < get_barrier_time() * 1000 ) {
          row["mockup"] = row["mockup"] + "*";
        }
      }
    } else {
      row["slowdown"] = "";
      row["mockup"] = "";
      row["mockup_median"] = "";
    }*/

    row["slowdown"] = "";
    row["mockup"] = "";
    row["mockup_median"] = "";
    res.add_row(row);
  }
  res.set_col_widths(col_widths);
  return res;
}