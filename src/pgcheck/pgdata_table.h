//
// Created by Max on 11/17/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGDATA_TABLE_H
#define REPROMPI_SRC_PGCHECK_PGDATA_TABLE_H

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>

class PGDataTable {

private:
  std::string mpi_name;
  std::vector<std::string> col_names;
  std::vector<int> col_widths;
  std::unordered_map<std::string, std::vector<std::string>> col_value_map;

public:
  PGDataTable(std::string mpi_name, std::vector<std::string> col_names);
  void add_row(std::unordered_map<std::string,std::string>& row_map);
  void set_col_widths(std::vector<int> widths);
  std::string get_mpi_name();
  std::vector<std::string> get_col_names();
  int get_col_width(int index);
  std::unordered_map<std::string, std::vector<std::string>> get_col_value_map();
  int get_col_size();
  std::vector<std::string> get_values_for_col_name(std::string key);
};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_TABLE_H
