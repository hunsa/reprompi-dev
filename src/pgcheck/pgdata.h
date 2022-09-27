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
  rapidcsv::Document csv;

public:
  PGData(std::string csv_string);
  std::vector<std::string> get_columns_names();


};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_H
