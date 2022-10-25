//
// Created by Sascha on 10/22/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_TTESTCOMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_TTESTCOMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class TTestComparer : public PGDataComparer {

public:
  TTestComparer(std::string mpi_coll_name, int nnodes, int ppn);
  PGCompareResults* get_results();
};

class TTestResults : public PGCompareResults {

private:
  std::string mpi_name;
  std::vector<std::string> col_names;
  std::unordered_map<std::string, std::vector<std::string>> col_value_map;

public:
  TTestResults(std::string mpi_name, std::vector<std::string> col_names);
  void add_row(std::unordered_map<std::string,std::string>& row_map);
  std::string get();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_TTESTCOMPARER_H
