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

#ifndef SRC_PGCHECK_PGDATA_PRINTER_H_
#define SRC_PGCHECK_PGDATA_PRINTER_H_

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <memory>

#include "constants.h"
#include "pgdata.h"
#include "comparer/comparer_factory.h"
#include "comparer/raw/raw_comparer.h"
#include "pgcheck_options.h"
#include "logger/logger.h"


class PGDataPrinter {
 private:
  /**
   * @return table formatted for txt or console as string
   */
  std::string table_to_clear_string(PGDataTable data_result);
  /**
   * @return table formatted for csv file as string
   */
  std::string table_to_csv_string(PGDataTable data_result);
  /**
   * writes text to new file with filename
   */
  void write_string_to_file(std::string text, std::string filename);
  /**
   * adds the result from collective to table
   */
  void add_table_to_merged_table(PGDataTable data_table, size_t merge_table_id);

 private:
  PGCheckOptions options;
  std::vector <PGDataTable> merged_table;

 public:
  PGDataPrinter() = default;
  /**
   * prints result from collective as txt or csv to file or console
   * @return 0 if print was successful
   */
  int print_collective(std::unique_ptr<PGDataComparer>& comparer, int comparer_type, size_t merge_table_id);

  /**
   * prints merged table as txt or csv to file or console
   * @return 0 if print was successful
   */
  int print_summary();

  void set_options(const PGCheckOptions &new_options);
};

#endif  // SRC_PGCHECK_PGDATA_PRINTER_H_
