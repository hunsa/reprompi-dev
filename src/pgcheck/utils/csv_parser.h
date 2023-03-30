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

#ifndef SRC_PGCHECK_UTILS_CSV_PARSER_H_
#define SRC_PGCHECK_UTILS_CSV_PARSER_H_

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <string>
#include <limits>
#include <sstream>
#include <fstream>
#include <set>

#include "csv_parser.h"
#include "../pgdata_table.h"
#include "../pgdata.h"
#include "../comparer/statistical_test/ttest.h"

class CSVParser {
 public:
  CSVParser() = default;
  PGDataTable parse_file(std::string csv_path);
};

#endif  // SRC_PGCHECK_UTILS_CSV_PARSER_H_
