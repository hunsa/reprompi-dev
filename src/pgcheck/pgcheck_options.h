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

#ifndef SRC_PGCHECK_PGCHECK_OPTIONS_H_
#define SRC_PGCHECK_PGCHECK_OPTIONS_H_

#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <filesystem>

#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include "comparer/comparer_factory.h"
#include "logger/logger.h"

class PGCheckOptions {
 private:
  bool merge_coll_tables = false;       // write additional file containing merged results
  bool csv = false;                     // write results in csv format to file
  bool verbose = false;                 // write information and results to console
  bool allow_mkdir = false;             // allow pgchecker to make directories
  std::vector<int> comparer_list = {};  // grouped comparer is default
  int test_type = 0;                    // default is t-test
  std::string input_file = "";
  std::string output_directory = "";
  std::string config_message = "";

 public:
  PGCheckOptions() = default;
  bool get_merge_coll_tables();
  bool get_print_to_csv();
  bool is_verbose();
  bool get_csv();
  int get_test_type();
  std::string get_config_message();
  std::string get_input_file();
  std::string get_output_directory();
  std::vector<int> get_comparer_list();

  /**
   * options are parsed
   * @return true if successful, otherwise false
   */
  bool parse(int argc, char *argv[], bool is_root);

  std::string get_usage_string();
};
#endif  // SRC_PGCHECK_PGCHECK_OPTIONS_H_
