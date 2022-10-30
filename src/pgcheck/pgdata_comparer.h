
#ifndef REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H

#include "pgdata.h"
#include "comparer/comparer_data.h"
#include "utils/statistics_utils.h"
#include "utils/statistics_utils.cpp"
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>

class PGDataComparer {

protected:
  std::string mpi_coll_name;
  int nnodes;  // number of nodes
  int ppn;     // number of processes per node
  std::unordered_map<std::string, PGData *> mockup2data;

public:

  PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn);

  void add_dataframe(std::string mockup_name, PGData *data);

  virtual std::string get_results() = 0;

};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
