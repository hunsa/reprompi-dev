
#include <vector>
#include <numeric>
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name) :
mpi_coll_name(mpi_coll_name)
{}

void PGDataComparer::add_dataframe(std::string mockup_name, PGData *data) {
  mockup2data.insert( {mockup_name, data} );
}

template<typename T>
static T mean(std::vector<T> v) {
  T sum = std::accumulate(v.begin(), v.end(), 0.0);
  return sum/v.size();
}

PGCompareResults PGDataComparer::get_results() {

  std::vector<std::string> col_names = { "mockup", "count", "runtime" };
  PGCompareResults res(mpi_coll_name, col_names);

  //std::cout << "get_results for: " << mpi_coll_name << std::endl;
  for(auto& mdata : mockup2data) {
    auto& data = mockup2data.at(mdata.first);
    //std::cout << "got data for: " << mdata.first << std::endl;
    for(auto& count : data->get_unique_counts()) {
      //std::cout << "count: " << count << std::endl;
      auto rts = data->get_runtimes_for_count(count);
      double mean_rts = mean(rts);
      std::unordered_map<std::string, std::string> row;
      row["mockup"] = mdata.first;
      row["count"] = std::to_string(count);
      row["runtime"] = std::to_string(mean_rts);
      res.add_row(row);
    }
  }

  std::cout << res << std::endl;

  return res;
}