//
// Created by Sascha on 9/26/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGDATA_H
#define REPROMPI_SRC_PGCHECK_PGDATA_H

#include <string>
#include <vector>
#include <limits>
#include "utils/rapidcsv.h"

class PGData {
  std::string mpi_coll_name;
  std::string mockup_name;
  rapidcsv::Document csv;

public:
  PGData(std::string mpi_coll_name, std::string mockup_name);

  int read_csv_from_file(std::string csv_path);

  std::vector<std::string> get_columns_names();

  std::vector<double> get_runtimes_for_count(int count);

  std::vector<int> get_unique_counts();
};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_H
