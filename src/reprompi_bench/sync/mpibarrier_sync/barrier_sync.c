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

#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/sync_info.h"

static inline double mpibarrier_get_normalized_time(double local_time) {
    return local_time;
}


static void mpibarrier_init_synchronization(const reprompib_sync_params_t* sync_params) {
    MPI_Barrier(MPI_COMM_WORLD);
}


static void mpibarrier_start_synchronization(void) {
    MPI_Barrier(MPI_COMM_WORLD);
#ifdef ENABLE_DOUBLE_BARRIER
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}


static int* mpibarrier_get_errorcodes(void) {
  return NULL;
}

static void mpibarrier_print_sync_parameters(FILE* f) {
    fprintf(f, "#@sync=MPI_Barrier\n");
#ifdef ENABLE_DOUBLE_BARRIER
    fprintf(f, "#@doublebarrier=true\n");
#endif
}


static void mpibarrier_init_module(int argc, char** argv) {
}


static void empty(void) {
}


void register_mpibarrier_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = "MPI_Barrier";
  sync_mod->sync_type = REPROMPI_SYNCTYPE_BARRIER;

  sync_mod->init_module = mpibarrier_init_module;
  sync_mod->cleanup_module = empty;

  sync_mod->init_sync = mpibarrier_init_synchronization;
  sync_mod->finalize_sync = empty;

  sync_mod->sync_clocks = empty;
  sync_mod->init_sync_round = empty;

  sync_mod->start_sync = mpibarrier_start_synchronization;
  sync_mod->stop_sync = empty;

  sync_mod->get_global_time = mpibarrier_get_normalized_time;
  sync_mod->get_errorcodes = mpibarrier_get_errorcodes;
  sync_mod->print_sync_info = mpibarrier_print_sync_parameters;
}



