
#include <iostream>
#include <unordered_map>
#include "pgcomparer_results.h"

PGCompareResults::PGCompareResults(std::string mpi_name, std::vector<std::string> col_names) :
  mpi_name(mpi_name), col_names(col_names)
{
}

std::ostream& operator<< (std::ostream& out, const PGCompareResults& pgres) {
  out << "MPI Collective: " << pgres.mpi_name << std::endl;
  return out;
}

void PGCompareResults::add_row(std::unordered_map<std::string,std::string> row_map) {
  for(auto& rows : row_map) {
    col_values[rows.first].push_back(rows.second);
  }
}