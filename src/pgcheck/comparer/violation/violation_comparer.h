//
// Created by Sascha on 10/22/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_VIOLATION_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_VIOLATION_COMPARER_H

#include "../../pgdata_comparer.h"
#include <string>

class ViolationComparer : public PGDataComparer {

private:
  int test_type;

public:
  ViolationComparer(int test_type, std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in t-test format
   */
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_VIOLATION_VIOLATION_COMPARER_H
