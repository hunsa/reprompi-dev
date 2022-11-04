//
// Created by Max on 11/04/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_DETAILED_TTEST_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_DETAILED_TTEST_COMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class DetailedTTestComparer : public PGDataComparer {

public:
  DetailedTTestComparer(std::string mpi_coll_name, int nnodes, int ppn);
  std::string get_results();
};

class DetailedTTestResults {

private:
  std::string mpi_name;
  std::vector<std::string> col_names;
  std::unordered_map<std::string, std::vector<std::string>> col_value_map;

public:
  DetailedTTestResults(std::string mpi_name, std::vector<std::string> col_names);
  void add_row(std::unordered_map<std::string,std::string>& row_map);
  std::string get();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_DETAILED_TTEST_COMPARER_H
