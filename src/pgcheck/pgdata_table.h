//
// Created by Max on 11/17/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGDATA_TABLE_H
#define REPROMPI_SRC_PGCHECK_PGDATA_TABLE_H

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <numeric>

class PGDataTable {

private:
  std::string mpi_name;
  std::vector<int> col_widths;
  std::vector <std::string> col_names;
  std::unordered_map <std::string, std::vector<std::string>> col_value_map;

public:
  PGDataTable() = default;

  PGDataTable(std::string mpi_name, std::vector <std::string> col_names);

  int get_col_width(int index);

  int get_col_size();

  std::string get_mpi_name();

  std::vector<int> get_col_widths();

  std::vector <std::string> get_col_names();

  std::vector <std::string> get_values_for_col_name(std::string key);

  std::unordered_map <std::string, std::vector<std::string>> get_col_value_map();

  void set_col_widths(std::vector<int> widths);

  void set_col_names(std::vector <std::string> names);

  /**
   * adds one row to table
   */
  void add_row(std::unordered_map <std::string, std::string> &row_map);

  /**
   * merges table with this table;
   * adds column collective name to table this
   */
  void add_table(PGDataTable table);

};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_TABLE_H
