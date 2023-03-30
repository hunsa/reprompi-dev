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

#ifndef SRC_PGCHECK_UTILS_ARGV_MANAGER_H_
#define SRC_PGCHECK_UTILS_ARGV_MANAGER_H_

#include <string>
#include <vector>

namespace argv {
  void compose_argv_vector(std::string prog_name,
                           std::string reprompi_params,
                           std::vector <std::string> &pgtunelib_params,
                           std::vector <std::string> &argv_vector);
  void convert_vector_to_argv_cstyle(std::vector <std::string> &argv_vector,
                                     int *argc_out, char ***argv_out);
  void free_argv_cstyle(int argc_test, char **argv_test);
}  // namespace argv

#endif  // SRC_PGCHECK_UTILS_ARGV_MANAGER_H_
