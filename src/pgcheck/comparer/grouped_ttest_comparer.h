//
// Created by Max on 10/30/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_GROUPED_TTEST_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_GROUPED_TTEST_COMPARER_H

#include "../pgdata_comparer.h"

class GroupedTTestComparer : public PGDataComparer {

public:
  GroupedTTestComparer(std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in grouped t-test format
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_GROUPED_TTEST_COMPARER_H
