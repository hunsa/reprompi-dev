//
// Created by Max on 10/30/22.
//

#include "default_comparer.h"

static const int col_widths[] = { 50, 10, 10 };

template<typename T>
static T mean(std::vector<T> v) {
  T sum = std::accumulate(v.begin(), v.end(), 0.0);
  return sum/v.size();
}

template<typename T>
static T median(std::vector<T> v) {
  sort(v.begin(), v.end());
  if (v.size() % 2 == 0) {
    return (v[v.size() / 2] + v[v.size() / 2 - 1]) / 2;
  } else {
    return v[v.size() / 2];
  }
}


DefaultResults::DefaultResults(std::string mpi_name, std::vector<std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names)
{
}

std::string DefaultResults::get() {
  std::stringstream res;

  res << "MPI Collective: " << this->mpi_name << "\n";

  int idx = 0;
  for(auto& colname : this->col_names) {
    res << std::setw(col_widths[idx++]) << colname << " ";
  }
  res << "\n";

  int nb_rows = this->col_value_map.at(this->col_names[0]).size();
  for(int i=0; i<nb_rows; i++) {
    idx = 0;
    for(auto& colname : this->col_names) {
      auto & values = this->col_value_map.at(colname);
      res << std::setw(col_widths[idx++]) << values[i] << " ";
    }
    res << "\n";
  }

  return res.str();
}

void DefaultResults::add_row(std::unordered_map<std::string,std::string>& row_map) {
  for(auto& rows : row_map) {
    //std::cout << "map: " << rows.first << " -> " << rows.second << std::endl;
    col_value_map[rows.first].push_back(rows.second);
  }
}


DefaultComparer::DefaultComparer(std::string mpi_coll_name, int nnodes, int ppn) :
  PGDataComparer(mpi_coll_name, nnodes, ppn)
{}

std::string DefaultComparer::get_results() {

  std::vector <std::string> col_names = {"mockup", "count", "runtime"};
  DefaultResults res(mpi_coll_name, col_names);

  std::cout << "nnodes: " << nnodes << " ppn: " << ppn << std::endl;
  //std::cout << "get_results for: " << mpi_coll_name << std::endl;
  for (auto &mdata: mockup2data) {
    auto &data = mockup2data.at(mdata.first);
    //std::cout << "got data for: " << mdata.first << std::endl;
    for (auto &count: data->get_unique_counts()) {
      //std::cout << "count: " << count << std::endl;
      auto rts = data->get_runtimes_for_count(count);
      double mean_rts = mean(rts);
      std::unordered_map <std::string, std::string> row;
      row["mockup"] = mdata.first;
      row["count"] = std::to_string(count);
      row["runtime"] = std::to_string(mean_rts);
      res.add_row(row);
    }
  }

  return res.get();
}