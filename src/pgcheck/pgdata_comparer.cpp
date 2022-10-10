
#include <vector>
#include <numeric>
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
mpi_coll_name(mpi_coll_name), nnodes(nnodes), ppn(ppn)
{}

void PGDataComparer::add_dataframe(std::string mockup_name, PGData *data) {
  mockup2data.insert( {mockup_name, data} );
}

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
static T standard_deviation(std::vector<T> v) {
    T sum = 0;
    T v_mean = mean(v);
    for (size_t idx = 0; idx < v.size(); idx++) {
      sum = sum + (v[idx] - v_mean) * (v[idx] - v_mean);
    }
    return sqrt(sum / v.size());
}

template<typename T>
static T t_test(std::vector<T> v, double default_mean) {
    double v_mean = mean(v);
    double v_standard_deviation = standard_deviation(v);
    return (v_mean - default_mean) / v_standard_deviation / sqrt(v.size());
}


PGCompareResults PGDataComparer::get_results() {

  std::vector<std::string> col_names = { "mockup", "count", "runtime" };
  PGCompareResults res(mpi_coll_name, col_names);

  std::cout << "nnodes: " << nnodes << " ppn: " << ppn << std::endl;
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

  return res;
}

PGCompareResults PGDataComparer::get_results_t_test() {

    std::vector<std::string> col_names = { "mockup", "count", "N", "ppn", "n", "runtime_mean", "runtime_median", "t_value", "critical_t_value", "violation"  };
    PGCompareResults res(mpi_coll_name, col_names);

    std::unordered_map<int, double> critical_t_values;
    critical_t_values.insert(std::make_pair(3, 2.919986));
    critical_t_values.insert(std::make_pair(4, 2.353363));
    critical_t_values.insert(std::make_pair(5, 2.131847));
    critical_t_values.insert(std::make_pair(6, 2.015048));
    critical_t_values.insert(std::make_pair(7, 1.943180));
    critical_t_values.insert(std::make_pair(8, 1.894579));
    critical_t_values.insert(std::make_pair(9, 1.859548));
    critical_t_values.insert(std::make_pair(10, 1.833113));
    critical_t_values.insert(std::make_pair(11, 1.812461));
    critical_t_values.insert(std::make_pair(12, 1.795885));
    critical_t_values.insert(std::make_pair(13, 1.782288));
    critical_t_values.insert(std::make_pair(14, 1.770933));
    critical_t_values.insert(std::make_pair(15, 1.761310));
    critical_t_values.insert(std::make_pair(16, 1.753050));
    critical_t_values.insert(std::make_pair(17, 1.745884));
    critical_t_values.insert(std::make_pair(18, 1.739607));
    critical_t_values.insert(std::make_pair(19, 1.734064));
    critical_t_values.insert(std::make_pair(20, 1.729133));
    critical_t_values.insert(std::make_pair(21, 1.724718));
    critical_t_values.insert(std::make_pair(22, 1.644854));


    std::unordered_map<int, double> default_data_results;
    auto& default_data = mockup2data.at("default");
    for(auto& count : default_data->get_unique_counts()) {
        auto rts_default = default_data->get_runtimes_for_count(count);
        default_data_results.insert(std::make_pair(count, mean(rts_default)));
    }

    for(auto& mdata : mockup2data) {
        auto& data = mockup2data.at(mdata.first);
        for(auto& count : data->get_unique_counts()) {

            auto rts = data->get_runtimes_for_count(count);

            double mean_rts = mean(rts);
            double median_rts = median(rts);
            double t_test_rts = t_test(rts, default_data_results.at(count));

            std::unordered_map<std::string, std::string> row;

            bool violation_rts = t_test_rts > critical_t_values.at(22);
            row["critical_t_value"] = std::to_string(critical_t_values.at(22));
            if(rts.size() < 22 && rts.size() > 2) {
                violation_rts = t_test_rts > critical_t_values.at(rts.size());
                row["critical_t_value"] = std::to_string(critical_t_values.at(rts.size()));
            }

            row["mockup"] = mdata.first;
            row["count"] = std::to_string(count);
            row["N"] = std::to_string(nnodes);
            row["ppn"] = std::to_string(ppn);
            row["n"] = std::to_string(rts.size());
            row["runtime_mean"] = std::to_string(mean_rts*1000);
            row["runtime_median"] = std::to_string(median_rts*1000);
            row["t_value"] = std::to_string(t_test_rts);
            row["violation"] = std::to_string(violation_rts);
            res.add_row(row);
        }
    }

    return res;
}