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

#ifndef SRC_PGCHECK_PGCHECK_INPUT_H_
#define SRC_PGCHECK_PGCHECK_INPUT_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

#include "utils/string_utils.h"

class PGInput {
 private:
  std::vector <std::string> mpi_collectives;
  std::vector <std::string> call_options;
 public:
  explicit PGInput(std::string input_file_name);
  int get_number_of_test_cases();
  std::string get_mpi_collective_for_case_id(int case_id);
  std::string get_call_options_for_case_id(int case_id);
};

#endif  // SRC_PGCHECK_PGCHECK_INPUT_H_
