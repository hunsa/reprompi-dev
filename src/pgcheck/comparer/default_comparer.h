//
// Created by Max on 10/30/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_DEFAULT_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_DEFAULT_COMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class DefaultComparer : public PGDataComparer {

public:
  DefaultComparer(std::string mpi_coll_name, int nnodes, int ppn);
  std::string get_results();
};

class DefaultResults {

private:
  std::string mpi_name;
  std::vector<std::string> col_names;
  std::unordered_map<std::string, std::vector<std::string>> col_value_map;

public:
  DefaultResults(std::string mpi_name, std::vector<std::string> col_names);
  void add_row(std::unordered_map<std::string,std::string>& row_map);
  std::string get();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_DEFAULT_COMPARER_H
