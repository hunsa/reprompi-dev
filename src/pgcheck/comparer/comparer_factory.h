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

#ifndef SRC_PGCHECK_COMPARER_COMPARER_FACTORY_H_
#define SRC_PGCHECK_COMPARER_COMPARER_FACTORY_H_

#include <string>

#include "simple_comparer.h"
#include "violation/violation_comparer.h"
#include "violation/detailed_violation_comparer.h"
#include "violation/grouped_violation_comparer.h"
#include "runtime/abs_runtime_comparer.h"
#include "runtime/rel_runtime_comparer.h"
#include "raw/raw_comparer.h"
#include "../pgdata_comparer.h"

class ComparerFactory {
public:
  static std::unique_ptr<PGDataComparer> create_comparer(int comparer_id, int test_type, std::string mpi_coll_name, int nnodes, int ppn);
};

#endif  // SRC_PGCHECK_COMPARER_COMPARER_FACTORY_H_
