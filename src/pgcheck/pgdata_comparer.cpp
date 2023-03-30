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

#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
    nnodes(nnodes), ppn(ppn), mpi_coll_name(mpi_coll_name) {}

bool PGDataComparer::has_barrier_time() {
  return barrier_time_s != CONSTANTS::NO_BARRIER_TIME_VALUE;
}

double PGDataComparer::get_barrier_time() {
  return barrier_time_s;
}

void PGDataComparer::add_data(std::unordered_map<std::string, PGData *> data) {
  mockup2data = data;
}

void PGDataComparer::set_barrier_time(double time_s) {
  barrier_time_s = time_s;
}
