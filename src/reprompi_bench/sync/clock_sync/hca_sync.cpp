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

#include <cstdio>
#include <string>
#include <cstdlib>
#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.h"
#include "reprompi_bench/misc.h"
//#include "reprompi_bench/sync/time_measurement.h"
//#include "reprompi_bench/sync/clock_sync/utils/sync_info.h"
//#include "reprompi_bench/sync/clock_sync/synchronization.h"

static const int HCA_WARMUP_ROUNDS = 5;

typedef struct {
    int n_fitpoints; /* --fitpoints */
    int n_exchanges; /* --exchanges */
} reprompi_hca_params_t;


// options specified from the command line
static reprompi_hca_params_t parameters;


static ClockSyncInterface* clock_sync;
static Clock* local_clock;
static Clock* global_clock;


static void synchronize_clocks(void) {
  global_clock = clock_sync->synchronize_all_clocks();
  //printf("clock: %20.9f   %20.9f\n", dynamic_cast<GlobalClockLM<RdtscpClock>*>(global_clock)->get_slope(),
  //    dynamic_cast<GlobalClockLM<RdtscpClock>*>(global_clock)->get_intercept());
}

static double get_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}

static void hca_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}



static void hca_common_print(FILE* f)
{
  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
}

static void hca_print_sync_parameters(FILE* f)
{
  hca_common_print(f);
  fprintf (f, "#@clocksync=HCA\n");
  fprintf(f, "#@hcasynctype=linear\n");
}

static void hca2_print_sync_parameters(FILE* f)
{
  hca_common_print(f);
  fprintf (f, "#@clocksync=HCA2\n");
  fprintf(f, "#@hcasynctype=logp\n");
}

static void hca3_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=HCA3\n");
}

static void hca2_init_module(int argc, char** argv) {
  //reprompib_sync_options_t sync_opts;
  //hca_parse_options(argc, argv, &sync_opts);

  global_clock = NULL;
  local_clock = new RdtscpClock();
  clock_sync = new HCA2ClockSync<PingpongClockOffsetAlg>(MPI_COMM_WORLD, local_clock);

  //clock_sync = new HCAClockSync<SKaMPIClockOffsetAlg>(MPI_COMM_WORLD, local_clock);
  //parameters.n_exchanges = sync_opts.n_exchanges;
 // parameters.n_fitpoints = sync_opts.n_fitpoints;
  //printf("HCA\n");

}

static void hca3_init_module(int argc, char** argv) {

  global_clock = NULL;
  local_clock = new RdtscpClock();
  clock_sync = new HCA3ClockSync<PingpongClockOffsetAlg>(MPI_COMM_WORLD, local_clock);

}


extern "C"
void register_hca_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA2").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_HCA2;
  sync_mod->init_module = hca2_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca2_print_sync_parameters;
}

extern "C"
void register_hca2_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA2").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_HCA2;
  sync_mod->init_module = hca2_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca2_print_sync_parameters;
}


extern "C"
void register_hca3_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA3").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_HCA3;
  sync_mod->init_module = hca3_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca3_print_sync_parameters;
}


