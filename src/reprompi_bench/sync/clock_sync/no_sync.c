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

#include "reprompi_bench/sync/clock_sync/synchronization.h"


static void empty(void) {
};

static void empty_init_module(int argc, char** argv) {
}

static double get_normalized_time(double local_time) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "WARNING: Global time is not defined.\n");
  }

  return local_time;
}


static void print_sync_parameters(FILE* f) {
    fprintf(f, "#@clocksync=None\n");
}



void register_no_clock_sync_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = "None";
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_NONE;

  sync_mod->init_module = empty_init_module;
  sync_mod->cleanup_module = empty;

  sync_mod->init_sync = empty;
  sync_mod->finalize_sync = empty;

  sync_mod->sync_clocks = empty;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = print_sync_parameters;
}


