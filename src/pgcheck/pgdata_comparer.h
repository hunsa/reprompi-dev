
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


class PGDataResults {

private:
  std::string mpi_name;
  std::vector<std::string> col_names;
  std::vector<int> col_widths;
  std::unordered_map<std::string, std::vector<std::string>> col_value_map;

public:
  PGDataResults(std::string mpi_name, std::vector<std::string> col_names);
  void add_row(std::unordered_map<std::string,std::string>& row_map);
  void set_col_widths(std::vector<int> widths);
  std::string get_mpi_name();
  std::vector<std::string> get_col_names();
  int get_col_width(int index);
  std::unordered_map<std::string, std::vector<std::string>> get_col_value_map();
  int get_col_size();
  std::vector<std::string> get_values_for_col_name(std::string key);
};

class PGDataComparer {

protected:
  std::string mpi_coll_name;
  int nnodes;  // number of nodes
  int ppn;     // number of processes per node
  std::unordered_map<std::string, PGData *> mockup2data;
  double barrier_time_s = -1.0;

public:

  PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn);
  virtual ~PGDataComparer() {};

  void add_dataframe(std::string mockup_name, PGData *data);

  void add_data(std::unordered_map<std::string, PGData *> data);

  virtual PGDataResults get_results() = 0;

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
