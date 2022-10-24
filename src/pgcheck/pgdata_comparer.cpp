
#include <vector>
#include <numeric>
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
mpi_coll_name(mpi_coll_name), nnodes(nnodes), ppn(ppn)
{}

void PGDataComparer::add_dataframe(std::string mockup_name, PGData *data) {
  mockup2data.insert( {mockup_name, data} );
}

const double PGDataComparer::critical_t_values[] = { 0, 6.314, 2.919986, 2.353363, 2.131847, 2.015048, 1.943180, 1.894579,1.859548, 1.833113,1.812461, 1.795885, 1.782288, 1.770933,1.761310,1.753050, 1.745884, 1.739607, 1.734064, 1.729133, 1.724718};
const double PGDataComparer::normal_distribution_value = 1.644854;

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

    std::vector<std::string> col_names = { "collective", "count", "N", "ppn", "n", "default_median", "slowdown", "mockup", "mockup_median" };
    PGCompareResults res(mpi_coll_name, col_names);
    std::unordered_map<int, std::pair<std::string, double>> fastest_mockup;

    std::unordered_map<int, StatisticValues> default_data_results;

    auto& default_data = mockup2data.at("default");
    for(auto& count : default_data->get_unique_counts()) {

        auto rts_default = default_data->get_runtimes_for_count(count);

        fastest_mockup.insert(std::make_pair(count, std::make_pair("", 0)));

        StatisticValues default_values;
        default_values.size = rts_default.size();
        default_values.mean = mean(rts_default);
        default_values.median = median(rts_default);
        default_values.variance = variance(rts_default);

        std::unordered_map<std::string, std::string> row;

        row["collective"] = mpi_coll_name;
        row["count"] = std::to_string(count);
        row["N"] = std::to_string(nnodes);
        row["ppn"] = std::to_string(ppn);
        row["n"] = std::to_string(default_values.size);
        row["default_median"] = std::to_string(default_values.median*1000);

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
            bool violation_rts = t_test_rts < -critical_rts;
            if(df <= 20 && df >= 1) {
                critical_rts = critical_t_values[df];
                violation_rts = t_test_rts < -critical_rts;
            }

            if(violation_rts == 1 && ((alternative_values.median < fastest_mockup.at(count).second) || fastest_mockup.at(count).second == 0)) {
                fastest_mockup[count].first = mdata.first;
                fastest_mockup[count].second = alternative_values.median;
            }
        }
    }

    for (auto& count: fastest_mockup) {

        auto& data = fastest_mockup.at(count.first);

        if(data.second != 0) {
            std::unordered_map<std::string, std::string> row;
            row["slowdown"] = std::to_string(default_data_results.at(count.first).median / data.second);
            row["mockup"] = data.first;
            row["mockup_median"] = std::to_string(data.second*1000);
            res.add_row(row);
        } else {
            std::unordered_map<std::string, std::string> row;
            row["slowdown"] = "";
            row["mockup"] = "";
            row["mockup_median"] = "";
            res.add_row(row);
        }
    }

    return res;
}