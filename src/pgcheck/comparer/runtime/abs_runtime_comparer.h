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

#ifndef SRC_PGCHECK_COMPARER_RUNTIME_ABS_RUNTIME_COMPARER_H_
#define SRC_PGCHECK_COMPARER_RUNTIME_ABS_RUNTIME_COMPARER_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "../../pgdata_comparer.h"

class AbsRuntimeComparer : public PGDataComparer {
 public:
  AbsRuntimeComparer(std::string mpi_coll_name, int nnodes, int ppn);
  /**
   * @return data table in message sizes to runtimes table
   */
  PGDataTable get_results();
};

#endif  // SRC_PGCHECK_COMPARER_RUNTIME_ABS_RUNTIME_COMPARER_H_
