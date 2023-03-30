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

#ifndef SRC_PGCHECK_PGTUNELIB_INTERFACE_H_
#define SRC_PGCHECK_PGTUNELIB_INTERFACE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <tuple>

class PGTuneLibInterface {
 private:
  std::vector <std::string> mpi_collectives;
  std::unordered_map <std::string, std::tuple<std::string, std::vector < std::string>>>
  mpi2module;

 public:
  explicit PGTuneLibInterface(std::string pgmpi_info_str);
  std::vector <std::string> get_available_mpi_collectives();
  std::string get_module_name_for_mpi_collectives(std::string mpi_coll_name);
  std::vector <std::string> get_available_implementations_for_mpi_collectives(std::string mpi_coll_name);
};

#endif  // SRC_PGCHECK_PGTUNELIB_INTERFACE_H_
