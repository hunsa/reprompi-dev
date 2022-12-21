//
// Created by Max on 11/04/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_DETAILED_VIOLATION_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_DETAILED_VIOLATION_COMPARER_H

#include "../../pgdata_comparer.h"

class DetailedViolationComparer : public PGDataComparer {

private:
  int test_type;

public:
  DetailedViolationComparer(int test_type, std::string mpi_coll_name, int nnodes, int ppn);

  /**
   * @return data table in detailed violation format
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_DETAILED_VIOLATION_COMPARER_H
