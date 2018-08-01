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
#include <assert.h>
#include <math.h>
#include <getopt.h>
#include <mpi.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/sync/process_sync/process_synchronization.h"
#include "reprompi_bench/sync/clock_sync/synchronization.h"

#include "round_sync_common.h"

//#define ZF_LOG_LEVEL ZF_LOG_INFO
//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static reprompib_sync_module_t* clock_sync_mod; /* pointer to current clock synchronization module */

// options specified from the command line
static reprompi_roundsync_bcast_params_t bcast_parameters;

static const int master_rank = 0;

// helper variables for the round-sync method
static    int invalid;
static double start_sync;
static double bcast_runtime = 0;

static void roundsync_init_sync_round(void) {
  // nothing to do
}

static void roundsync_start_synchronization(void) {
  int is_first = 1;
  double global_time;
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  MPI_Barrier(MPI_COMM_WORLD);
  if (my_rank == master_rank) {
    start_sync = get_time() + bcast_runtime * bcast_parameters.bcast_multiplier;
  }
  MPI_Bcast(&start_sync, 1, MPI_DOUBLE, master_rank, MPI_COMM_WORLD);
  global_time = clock_sync_mod->get_global_time(get_time());
  ZF_LOGV("[rank %d] current_time=%20.10f need_to_wait_us=%f", my_rank, global_time, 1e6*(start_sync-global_time));

  while (1) {
    global_time = clock_sync_mod->get_global_time(get_time());

    if (global_time >= start_sync) {
      if (is_first == 1) {
        invalid = REPROMPI_INVALID_MEASUREMENT;
      }
      break;
    }
    is_first = 0;
  }
}

static int roundsync_stop_synchronization(void) {
  //double global_time;
  int current_meas_invalid = REPROMPI_CORRECT_MEASUREMENT;

  MPI_Allreduce(&invalid, &current_meas_invalid, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  //global_time = get_time();
  return current_meas_invalid;
}


static void roundsync_init_module(int argc, char** argv, reprompib_sync_module_t* clock_sync) {
  roundsync_parse_bcast_options(argc, argv, &bcast_parameters);

  if (clock_sync->clocksync == REPROMPI_CLOCKSYNC_NONE) {
    reprompib_print_error_and_exit("Cannot use the round-sync process synchronization with the selected clock synchronization method (use \"--clock-sync\" to change it)");
  }

  clock_sync_mod = clock_sync;
  bcast_runtime = measure_bcast_runtime(MPI_COMM_WORLD, &bcast_parameters);
}


static void roundsync_cleanup_module(void) {
}

static int* roundsync_get_errorcodes(void) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "WARNING: Measurement errorcodes are not defined for the round-sync synchronization method.\n");
  }
  return NULL;
}

static void roundsync_init_synchronization(const reprompib_sync_params_t* init_params) {
  invalid = REPROMPI_CORRECT_MEASUREMENT;
}

static void roundsync_finalize_synchronization(void) {
}


static void roundsync_sync_print(FILE* f)
{
  fprintf(f, "#@procsync=roundsync\n");
  fprintf(f, "#@bcast_nrep=%ld\n", bcast_parameters.bcast_n_rep);
  fprintf(f, "#@bcast_runtime_s=%.10f\n", bcast_runtime);
}

void register_roundsync_module(reprompib_proc_sync_module_t *sync_mod) {
  sync_mod->name = "roundsync";
  sync_mod->procsync = REPROMPI_PROCSYNC_ROUNDSYNC;

  sync_mod->init_module = roundsync_init_module;
  sync_mod->cleanup_module = roundsync_cleanup_module;

  sync_mod->init_sync = roundsync_init_synchronization;
  sync_mod->finalize_sync = roundsync_finalize_synchronization;

  sync_mod->init_sync_round = roundsync_init_sync_round;
  sync_mod->start_sync = roundsync_start_synchronization;
  sync_mod->stop_sync = roundsync_stop_synchronization;

  sync_mod->get_errorcodes = roundsync_get_errorcodes;
  sync_mod->print_sync_info = roundsync_sync_print;
}



