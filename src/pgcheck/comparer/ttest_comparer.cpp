//
// Created by Sascha on 10/22/22.
//

#include "ttest_comparer.h"
#include <numeric>

static const double critical_t_values[] = { 0, 6.314, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,1.859548, 1.833113,1.812461, 1.795885, 1.782288, 1.770933,1.761310,1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718};
static const double normal_distribution_value = 1.644854;

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

template<typename T>
static T variance(std::vector<T> v) {
  const size_t v_size = v.size();
  const T v_mean = mean(v);

  auto variance_function = [&v_mean, &v_size](T accumulator, const T& value) {
    return accumulator + ((value - v_mean) * (value - v_mean) / (v_size - 1));
  };

  return std::accumulate(v.begin(), v.end(), 0.0, variance_function);
}

static double t_test(StatisticValues alternative_values, StatisticValues default_values) {
  double alternative_df = alternative_values.size-1;
  double default_df = default_values.size-1;
  double standard_error = sqrt((alternative_df*alternative_values.variance+default_df*default_values.variance)/(alternative_df+default_df));
  return sqrt((alternative_values.size*default_values.size)/(alternative_values.size+default_values.size)) * ((alternative_values.mean-default_values.mean) / standard_error);
}


TTestResults::TTestResults(std::string mpi_name, std::vector<std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names)
{
}

std::string TTestResults::get() {
  std::string res = "";

  res.append("MPI Collective: " + this->mpi_name + "\n");

  for(auto& colname : this->col_names) {
    res.append(colname + " ");
  }
  res.append("\n");

  int nb_rows = this->col_value_map.at(this->col_names[0]).size();
  for(int i=0; i<nb_rows; i++) {
    for(auto& colname : this->col_names) {
      auto & values = this->col_value_map.at(colname);
      res.append(values[i] + " ");
    }
    res.append("\n");
  }

  return res;
}

void TTestResults::add_row(std::unordered_map<std::string,std::string>& row_map) {
  for(auto& rows : row_map) {
    //std::cout << "map: " << rows.first << " -> " << rows.second << std::endl;
    col_value_map[rows.first].push_back(rows.second);
  }
}


TTestComparer::TTestComparer(std::string mpi_coll_name, int nnodes, int ppn) :
  PGDataComparer(mpi_coll_name, nnodes, ppn)
{}

PGCompareResults* TTestComparer::get_results() {

  std::vector<std::string> col_names = { "mockup", "count", "N", "ppn", "n", "runtime_mean", "runtime_median", "t_value", "critical_t_value", "violation"  };
  TTestResults* res = new TTestResults(mpi_coll_name, col_names);

  std::unordered_map<int, StatisticValues> default_data_results;

  auto& default_data = mockup2data.at("default");
  for(auto& count : default_data->get_unique_counts()) {

    auto rts_default = default_data->get_runtimes_for_count(count);

    StatisticValues default_values;
    default_values.size = rts_default.size();
    default_values.mean = mean(rts_default);
    default_values.median = median(rts_default);
    default_values.variance = variance(rts_default);

    std::unordered_map<std::string, std::string> row;
    row["mockup"] = "default";
    row["count"] = std::to_string(count);
    row["N"] = std::to_string(nnodes);
    row["ppn"] = std::to_string(ppn);
    row["n"] = std::to_string(default_values.size);
    row["runtime_mean"] = std::to_string(default_values.mean*1000);
    row["runtime_median"] = std::to_string(default_values.median*1000);
    row["critical_t_value"] = "";
    row["t_value"] = "";
    row["violation"] = "";

    res->add_row(row);
    default_data_results.insert(std::make_pair(count, default_values));

  }

  for(auto& mdata : mockup2data) {

    if (mdata.first == "default") {
      continue;
    }

    auto& data = mockup2data.at(mdata.first);
    for(auto& count : data->get_unique_counts()) {

      auto rts = data->get_runtimes_for_count(count);

      StatisticValues alternative_values;
      alternative_values.size = rts.size();
      alternative_values.mean = mean(rts);
      alternative_values.median = median(rts);
      alternative_values.variance = variance(rts);

      int df = alternative_values.size+default_data_results.at(count).size-2;
      double t_test_rts = t_test(alternative_values, default_data_results.at(count));
      double critical_rts = normal_distribution_value;
      bool violation_rts = t_test_rts < -critical_rts;
      if(df <= 20 && df >= 1) {
        critical_rts = critical_t_values[df];
        violation_rts = t_test_rts < -critical_rts;
      }

      std::unordered_map<std::string, std::string> row;
      row["mockup"] = mdata.first;
      row["count"] = std::to_string(count);
      row["N"] = std::to_string(nnodes);
      row["ppn"] = std::to_string(ppn);
      row["n"] = std::to_string(alternative_values.size);
      row["runtime_mean"] = std::to_string(alternative_values.mean*1000);
      row["runtime_median"] = std::to_string(alternative_values.median*1000);
      row["t_value"] = std::to_string(t_test_rts);
      row["critical_t_value"] = std::to_string(critical_rts);
      row["violation"] = std::to_string(violation_rts);
      res->add_row(row);
    }
  }

  return res;
}