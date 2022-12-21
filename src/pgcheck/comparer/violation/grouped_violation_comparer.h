//
// Created by Max on 10/30/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_GROUPED_VIOLATION_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_GROUPED_VIOLATION_COMPARER_H

#include "../../pgdata_comparer.h"

class GroupedViolationComparer : public PGDataComparer {

private:
  int test_type;

public:
  GroupedViolationComparer(int test_type, std::string mpi_coll_name, int nnodes, int ppn);

  /**
   * @return data table in grouped violation format
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_GROUPED_VIOLATION_COMPARER_H
