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

#ifndef SRC_PGCHECK_PGDATA_H_
#define SRC_PGCHECK_PGDATA_H_

#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>

#include "utils/string_utils.h"
#include "utils/csv_parser.h"
#include "pgdata_table.h"

class PGData {
 private:
  std::string mpi_coll_name;
  std::string mockup_name;
  PGDataTable table;
 public:
  PGData(std::string mpi_coll_name, std::string mockup_name);
  int read_csv_from_file(std::string csv_path);
  std::string get_mockup_name();
  std::vector <std::string> get_columns_names();
  std::vector<int> get_unique_counts();
  std::vector<double> get_runtimes_for_count(int count);
};

#endif  // SRC_PGCHECK_PGDATA_H_
