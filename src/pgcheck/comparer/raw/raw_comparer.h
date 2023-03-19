//
// Created by Max on 03/19/23.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_RUNTIME_RAW_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_RUNTIME_RAW_COMPARER_H

#include "../../pgdata_comparer.h"
#include <string>

class RawComparer : public PGDataComparer {

public:
  RawComparer(std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in message sizes to runtimes table
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_RUNTIME_RAW_COMPARER_H
