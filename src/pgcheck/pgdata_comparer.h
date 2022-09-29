
#ifndef REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H

#include <vector>
#include <unordered_map>
#include "pgdata.h"
#include "pgcomparer_results.h"

class PGDataComparer {

private:
  std::string mpi_coll_name;
  std::unordered_map<std::string, PGData*> mockup2data;

public:

  PGDataComparer(std::string mpi_coll_name);
  void add_dataframe(std::string mockup_name, PGData *data);
  PGCompareResults get_results();

};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_COMPARER_H
