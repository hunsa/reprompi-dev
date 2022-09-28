
#ifndef REPROMPI_SRC_PGCHECK_PGCOMPARER_RESULTS_H
#define REPROMPI_SRC_PGCHECK_PGCOMPARER_RESULTS_H

#include <string>
#include <vector>

class PGCompareResults {

private:
  std::string mpi_name;
  std::vector<std::string> col_names;
  std::unordered_map<std::string, std::vector<std::string>> col_values;

public:
  PGCompareResults(std::string mpi_name, std::vector<std::string> col_names);
  friend std::ostream& operator<< (std::ostream& out, const PGCompareResults& pgres);
  void add_row(std::unordered_map<std::string,std::string> row_map);
};


#endif //REPROMPI_SRC_PGCHECK_PGCOMPARER_RESULTS_H
