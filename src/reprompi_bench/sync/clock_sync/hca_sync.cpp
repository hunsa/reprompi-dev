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
#include "reprompi_bench/sync/clock_sync/clock_sync_lib.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClock.h"
#include "reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_loader.hpp"

#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"

#include "reprompi_bench/sync/clock_sync/sync_methods/HCAClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.h"
#include "reprompi_bench/misc.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

//typedef struct {
//    int n_fitpoints; /* --fitpoints */
//    int n_exchanges; /* --exchanges */
//} reprompi_hca_params_t;


//// options specified from the command line
//static reprompi_hca_params_t parameters;


static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void synchronize_clocks(void) {
  global_clock = clock_sync->synchronize_all_clocks(MPI_COMM_WORLD, *(local_clock));
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
//  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
//  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
}

/******************************
 *
 * HCA
 *
 */

static void hca_print_sync_parameters(FILE* f)
{
  hca_common_print(f);
  fprintf (f, "#@clocksync=HCA\n");
  fprintf(f, "#@hcasynctype=linear\n");
}


static void hca_init_module(int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync("alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really an HCA instance
    if( dynamic_cast<HCAClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type HCA. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default hca clock sync");
    clock_sync = new HCAClockSync(new PingpongClockOffsetAlg(100,100), 1000);
  }
}


/******************************
 *
 * HCA2
 *
 */

static void hca2_print_sync_parameters(FILE* f)
{
  hca_common_print(f);
  fprintf(f, "#@clocksync=HCA2\n");
  fprintf(f, "#@hcasynctype=logp\n");
}


static void hca2_init_module(int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync("alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really an HCA2 instance
    if( dynamic_cast<HCA2ClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type HCA2. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default hca2 clock sync");
    clock_sync = new HCA2ClockSync(new PingpongClockOffsetAlg(100,100), 1000, false);
  }
}


/******************************
 *
 * HCA3
 *
 */

static void hca3_init_module(int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync("alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really an HCA3 instance
    if( dynamic_cast<HCA3ClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type HCA3. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default hca3 clock sync");
    clock_sync = new HCA3ClockSync(new PingpongClockOffsetAlg(100,100), 1000, false);
  }


}

static void hca3_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=HCA3\n");
}


extern "C"
void register_hca_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("HCA").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_HCA;
  sync_mod->init_module = hca_init_module;
  sync_mod->cleanup_module = hca_cleanup_module;
  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = hca_print_sync_parameters;
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


