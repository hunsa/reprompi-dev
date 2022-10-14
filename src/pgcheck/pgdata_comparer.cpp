
#include <vector>
#include <numeric>
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
mpi_coll_name(mpi_coll_name), nnodes(nnodes), ppn(ppn)
{}

void PGDataComparer::add_dataframe(std::string mockup_name, PGData *data) {
  mockup2data.insert( {mockup_name, data} );
}

const double PGDataComparer::critical_t_values[] = { 0, 12.706205, 4.302653, 3.182446, 2.776445, 2.570582, 2.446912, 2.364624,2.306004, 2.262157,2.228139, 2.200985, 2.178813, 2.160369,2.144787,2.131450, 2.119905, 2.109816, 2.100922, 2.093024, 2.085963};
const double PGDataComparer::normal_distribution_value = 1.959964;

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
    T sum = 0;
    T v_mean = mean(v);
    for (size_t idx = 0; idx < v.size(); idx++) {
      sum = sum + (v[idx] - v_mean) * (v[idx] - v_mean);
    }

    double standard_deviation = sqrt(sum / v.size());
    return pow(standard_deviation, 2);
}

static double t_test(StatisticValues alternative_values, StatisticValues default_values) {
    double alternative_df = alternative_values.size-1;
    double default_df = default_values.size-1;
    double standard_error = sqrt((alternative_df*alternative_values.variance+default_df*default_values.variance)/(alternative_df+default_df));
    return sqrt((alternative_values.size*default_values.size)/(alternative_values.size+default_values.size)) * ((alternative_values.mean-default_values.mean) / standard_error);
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

        res.add_row(row);
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
            bool violation_rts = t_test_rts < -critical_rts || t_test_rts > critical_rts;
            if(df <= 20 && df >= 1) {
                critical_rts = critical_t_values[df];
                violation_rts = t_test_rts > critical_rts;
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
            res.add_row(row);
        }
    }

    return res;
}