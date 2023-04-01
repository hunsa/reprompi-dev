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

#include "csv_parser.h"

PGDataTable CSVParser::parse_file(std::string csv_path) {
  PGDataTable table_res{};

  std::ifstream infile(csv_path);
  std::string line;

  std::vector <std::string> col_names = {"test", "nrep", "count", "runtime_sec"};
  table_res.set_column_names(col_names);
  bool first = true;

  while (std::getline(infile, line)) {
    std::unordered_map <std::string, std::string> row;

    if (line[0] != '#') {
      if (first) {
        first = false;
        std::getline(infile, line);
      }

      std::istringstream iss(line);
      std::string token;
      int col_idx = 0;

      while (iss >> token) {
        row[col_names[col_idx++]] = token;
      }
    }
    table_res.add_row(row);
  }
  return table_res;
}
