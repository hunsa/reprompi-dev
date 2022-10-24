
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include "pgcomparer_results.h"

PGCompareResults::PGCompareResults(std::string mpi_name, std::vector<std::string> col_names) :
  mpi_name(mpi_name), col_names(col_names)
{
}

const int PGCompareResults::col_widths[] = { 15, 15, 5, 5, 5, 15, 10, 50, 15 };

std::ostream& operator<< (std::ostream& out, const PGCompareResults& pgres) {
  out << "MPI Collective: " << pgres.mpi_name << std::endl;

  int idx = 0;
  for(auto& colname : pgres.col_names) {
    out << std::setw(PGCompareResults::col_widths[idx++]) << colname << " ";
  }
  out << std::endl;

  int nb_rows = pgres.col_value_map.at(pgres.col_names[0]).size();
  for(int i=0; i<nb_rows; i++) {
    idx = 0;
    for(auto& colname : pgres.col_names) {
      auto & values = pgres.col_value_map.at(colname);
      //out << colname << "$" << values.size() << "$" << " ";
      out << std::setw(PGCompareResults::col_widths[idx++]) << values[i] << " ";
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