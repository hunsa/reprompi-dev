//
// Created by Sascha on 11/3/22.
//

#ifndef REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_FACTORY_H
#define REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_FACTORY_H

#include "../pgdata_comparer.h"

class ComparerFactory {
public:
  static PGDataComparer* create_comparer(int comparer_id, int test_type, std::string mpi_coll_name, int nnodes, int ppn);
};

#endif //REPROMPI_DEV_SRC_PGCHECK_COMPARER_COMPARER_FACTORY_H
