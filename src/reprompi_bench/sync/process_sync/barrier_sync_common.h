/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
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

#ifndef REPROMPI_SRC_REPROMPI_BENCH_SYNC_PROCESS_SYNC_BARRIER_SYNC_COMMON_H
#define REPROMPI_SRC_REPROMPI_BENCH_SYNC_PROCESS_SYNC_BARRIER_SYNC_COMMON_H

#include "reprompi_bench/sync/process_sync/process_synchronization.h"

void reprompi_register_common_barrier_functions(reprompib_proc_sync_module_t *sync_mod);

#endif //REPROMPI_SRC_REPROMPI_BENCH_SYNC_PROCESS_SYNC_BARRIER_SYNC_COMMON_H
