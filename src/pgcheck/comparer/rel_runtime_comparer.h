//
// Created by Max on 12/07/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_REL_RUNTIME_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_REL_RUNTIME_COMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class RelRuntimeComparer : public PGDataComparer {

public:
  RelRuntimeComparer(std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in message sizes to runtimes relative to default
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_REL_RUNTIME_COMPARER_H
