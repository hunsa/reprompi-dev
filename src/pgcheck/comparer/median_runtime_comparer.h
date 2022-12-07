//
// Created by Max on 12/07/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_MEDIAN_RUNTIME_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_MEDIAN_RUNTIME_COMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class MedianRuntimeComparer : public PGDataComparer {

public:
  MedianRuntimeComparer(std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in message sizes to runtimes table
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_MEDIAN_RUNTIME_COMPARER_H
