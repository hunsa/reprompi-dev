
#include <iostream>
#include <unordered_map>
#include "pgcomparer_results.h"

PGCompareResults::PGCompareResults(std::string mpi_name, std::vector<std::string> col_names) :
  mpi_name(mpi_name), col_names(col_names)
{
}

std::ostream& operator<< (std::ostream& out, const PGCompareResults& pgres) {
  out << "MPI Collective: " << pgres.mpi_name << std::endl;

  for(auto& colname : pgres.col_names) {
    out << colname << " ";
  }
  out << std::endl;

  int nb_rows = pgres.col_value_map.at(pgres.col_names[0]).size();
  for(int i=0; i<nb_rows; i++) {
    for(auto& colname : pgres.col_names) {
      auto & values = pgres.col_value_map.at(colname);
      //out << colname << "$" << values.size() << "$" << " ";
      out << values[i] << " ";
    }
    out << std::endl;
  }

  return out;
}

void PGCompareResults::add_row(std::unordered_map<std::string,std::string>& row_map) {
  for(auto& rows : row_map) {
    //std::cout << "map: " << rows.first << " -> " << rows.second << std::endl;
    col_value_map[rows.first].push_back(rows.second);
  }
}