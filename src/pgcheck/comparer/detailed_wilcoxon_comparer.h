//
// Created by Max on 12/17/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_DETAILED_WILCOXON_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_DETAILED_WILCOXON_COMPARER_H

#include "../pgdata_comparer.h"
#include "statistical_test/wilcoxon.h"

class DetailedWilcoxonComparer : public PGDataComparer {

public:
  DetailedWilcoxonComparer(std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in detailed t-test format
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_DETAILED_WILCOXON_COMPARER_H
