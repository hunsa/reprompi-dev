//
// Created by Sascha on 10/22/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_TTEST_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_TTEST_COMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class TTestComparer : public PGDataComparer {

public:
  TTestComparer(std::string mpi_coll_name, int nnodes, int ppn);
  PGDataTable get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_TTEST_COMPARER_H
