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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include "mpits.h"

#include "reprompi_bench/sync/process_sync/process_synchronization.h"


static int* barrier_get_errorcodes(void) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "WARNING: Measurement errorcodes are not defined for barrier-based synchronization.\n");
  }

  return NULL;
}


static void barrier_init_module(int argc, char** argv, mpits_clocksync_t* clock_sync) {
};

static void barrier_init_synchronization(const reprompib_sync_params_t* init_params) {
}

static void empty(void) {
};

static int stop_sync(MPI_Comm comm) {
  return REPROMPI_CORRECT_MEASUREMENT;
};


void reprompi_register_common_barrier_functions(reprompib_proc_sync_module_t *sync_mod) {
  sync_mod->init_module = barrier_init_module;
  sync_mod->cleanup_module = empty;

  sync_mod->init_sync = barrier_init_synchronization;
  sync_mod->stop_sync = stop_sync;
  sync_mod->finalize_sync = empty;
  sync_mod->init_sync_round = empty;
  sync_mod->get_errorcodes = barrier_get_errorcodes;
}
