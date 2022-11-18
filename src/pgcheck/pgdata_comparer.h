
#ifndef REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H

#include "pgdata.h"
#include "pgdata_table.h"
#include "comparer/comparer_data.h"
#include "utils/statistics_utils.h"
#include "utils/statistics_utils.cpp"
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>

class PGDataComparer {

protected:
  int nnodes;
  int ppn;
  double barrier_time_s = -1.0;
  std::string mpi_coll_name;
  std::unordered_map<std::string, PGData *> mockup2data;

public:

  PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn);
  virtual ~PGDataComparer() {};

  void add_dataframe(std::string mockup_name, PGData *data);

  void add_data(std::unordered_map<std::string, PGData *> data);

  virtual PGDataTable get_results() = 0;

  /**
   * adds the mean time for MPI_Barrier in seconds
   */
  void set_barrier_time(double time_s);

  /**
   *
   * @return saved MPI_Barrier time in seconds
   */
  double get_barrier_time();

  /**
   *
   * @return true if barrier time has been set before
   */
  bool has_barrier_time();

};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
