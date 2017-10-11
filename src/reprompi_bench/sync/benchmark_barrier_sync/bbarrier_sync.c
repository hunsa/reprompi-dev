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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "reprompi_bench/sync/sync_info.h"
#include "reprompi_bench/sync/synchronization.h"

static inline double bbarrier_get_normalized_time(double local_time) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "WARNING: Global time is not defined for barrier-based synchronization.\n");
  }

  return local_time;
}

static int* bbarrier_get_errorcodes(void) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "WARNING: Measurement errorcodes are not defined for barrier-based synchronization.\n");
  }

  return NULL;
}

static void bbarrier_init_synchronization(const reprompib_sync_params_t* sync_params) {
}


void dissemination_barrier(void) {
    int my_rank, np, send_rank, recv_rank;
    int i, nrounds;
    MPI_Status status;
    int send_value = 1;
    int recv_value = 1;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    nrounds = ceil(log2((double) np));

    for (i = 0; i < nrounds; i++) {
        send_rank = (my_rank + (1<<i)) % np;
        recv_rank = (my_rank - (1 << i) + np) % np;

        //printf("[%d] Sending from %d to %d; receive from %d\n", i, my_rank, send_rank, recv_rank);
        MPI_Sendrecv(&send_value, 1, MPI_INT, send_rank, 0,
                &recv_value, 1, MPI_INT, recv_rank, 0,
                MPI_COMM_WORLD, &status);
    }

}


static void bbarrier_start_synchronization(void) {
    dissemination_barrier();
#ifdef ENABLE_DOUBLE_BARRIER
    dissemination_barrier();
#endif
}

static void empty(void) {
}


static void bbarrier_print_sync_parameters(FILE* f) {
    fprintf(f, "#@sync=BBarrier\n");
#ifdef ENABLE_DOUBLE_BARRIER
    fprintf(f, "#@doublebarrier=true\n");
#endif
}


static void bbarrier_init_module(int argc, char** argv) {
}


void register_dissem_barrier_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = "Dissem_Barrier";
  sync_mod->sync_type = REPROMPI_SYNCTYPE_BARRIER;

  sync_mod->init_module = bbarrier_init_module;
  sync_mod->cleanup_module = empty;

  sync_mod->init_sync = bbarrier_init_synchronization;
  sync_mod->finalize_sync = empty;
  sync_mod->sync_clocks = empty;
  sync_mod->init_sync_round = empty;

  sync_mod->start_sync = bbarrier_start_synchronization;
  sync_mod->stop_sync = empty;

  sync_mod->get_global_time = bbarrier_get_normalized_time;
  sync_mod->get_errorcodes = bbarrier_get_errorcodes;
  sync_mod->print_sync_info = bbarrier_print_sync_parameters;
}






