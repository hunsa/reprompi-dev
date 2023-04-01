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

#include "comparer_factory.h"

std::unique_ptr<PGDataComparer>
ComparerFactory::create_comparer(int comparer_id, int test_type, std::string mpi_coll_name, int nnodes, int ppn) {
  std::unique_ptr<PGDataComparer> comparer;
  switch (comparer_id) {
    case 0:
      comparer = std::make_unique<SimpleComparer>(mpi_coll_name, nnodes, ppn);
      break;
    case 1:
      comparer = std::make_unique<AbsRuntimeComparer>(mpi_coll_name, nnodes, ppn);
      break;
    case 2:
      comparer = std::make_unique<RelRuntimeComparer>(mpi_coll_name, nnodes, ppn);
      break;
    case 3:
      comparer = std::make_unique<ViolationComparer>(test_type, mpi_coll_name, nnodes, ppn);
      break;
    case 4:
      comparer = std::make_unique<DetailedViolationComparer>(test_type, mpi_coll_name, nnodes, ppn);
      break;
    case 5:
      comparer = std::make_unique<GroupedViolationComparer>(test_type, mpi_coll_name, nnodes, ppn);
      break;
    default:
    case 6:
      comparer = std::make_unique<RawComparer>(mpi_coll_name, nnodes, ppn);
      break;
  }
  return comparer;
}

int ComparerFactory::get_number_of_comparers() {
  return 7;
}
