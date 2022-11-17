//
// Created by Max on 10/30/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_DEFAULT_COMPARER_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_DEFAULT_COMPARER_H

#include "../pgdata_comparer.h"
#include <string>

class SimpleComparer : public PGDataComparer {

public:
  SimpleComparer(std::string mpi_coll_name, int nnodes, int ppn);
  PGDataResults get_results();
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_DEFAULT_COMPARER_H
