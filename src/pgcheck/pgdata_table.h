/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#ifndef SRC_PGCHECK_PGDATA_TABLE_H_
#define SRC_PGCHECK_PGDATA_TABLE_H_

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <string>
#include <iostream>
#include <numeric>

#include <stdio.h>

class PGDataTable {
 private:
  std::string title;
  std::vector<int> col_widths;
  std::vector <std::string> column_names;
  std::unordered_map <std::string, std::vector<std::string>> col_value_map;

 public:
  PGDataTable() = default;
  PGDataTable(std::string title, std::vector <std::string> column_names);
  int get_col_width(int index);
  int get_col_size();
  std::string get_title();
  std::string get_values_col_row(std::string col, int row);
  std::vector<int> get_col_widths();
  std::vector <std::string> get_column_names();
  std::vector <std::string> get_values_for_col_name(std::string key);
  std::unordered_map <std::string, std::vector<std::string>> get_col_value_map();
  void set_col_widths(std::vector<int> widths);
  void set_column_names(std::vector <std::string> names);
  PGDataTable get_violation_table();
  /**
   * adds one row to table
   */
  void add_row(const std::unordered_map <std::string, std::string> &row_map);
  /**
   * merges table with this table;
   * adds column collective name to table this
   */
  void add_table(PGDataTable table);
  /**
 * @return table formatted for txt or console as string
 */
  std::string to_string();
  /**
   * @return table formatted for csv file as string
   */
  std::string to_csv_string();
};

#endif  // SRC_PGCHECK_PGDATA_TABLE_H_
