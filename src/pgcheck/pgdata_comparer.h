
#ifndef REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H

#include "utils/statistics_utils.h"
#include "utils/statistics_utils.cpp"
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include "pgdata.h"

struct StatisticValues {
  int size;
  double mean;
  double median;
  double variance;
  std::string mockup;
  double mockup_median;
};

class PGDataComparer {

protected:
  std::string mpi_coll_name;
  int nnodes;  // number of nodes
  int ppn;     // number of processes per node
//  const static double critical_t_values[];
//  const static double normal_distribution_value;
  std::unordered_map<std::string, PGData*> mockup2data;

public:

  PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn);
  void add_dataframe(std::string mockup_name, PGData *data);
  virtual std::string get_results() = 0;

};


#endif //REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
