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

#ifndef SRC_PGCHECK_PGDATA_COMPARER_H_
#define SRC_PGCHECK_PGDATA_COMPARER_H_

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <string>

#include "constants.h"
#include "pgdata.h"
#include "pgdata_table.h"
#include "comparer/comparer_data.h"
#include "utils/statistics_utils.h"

class PGDataComparer {
 protected:
  int nnodes;
  int ppn;
  double barrier_time_s = -1.0;
  std::string mpi_coll_name;
  std::unordered_map<std::string, PGData *> mockup2data;

 public:
  PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn);

  virtual ~PGDataComparer() {}
  /**
   * adds map of data to this
   */
  void add_data(std::unordered_map<std::string, PGData *> data);
  virtual PGDataTable get_results() = 0;
  /**
   *
   * @return true if barrier time has been set before
   */
  bool has_barrier_time();
  /**
   *
   * @return saved MPI_Barrier time in seconds
   */
  double get_barrier_time();
  /**
   * adds the mean time for MPI_Barrier in seconds
   */
  void set_barrier_time(double time_s);
};

#endif  // SRC_PGCHECK_PGDATA_COMPARER_H_
