/*  ReproMPI Benchmark
 *
 * Copyright 2003-2008 Werner Augustin, Lars Diesselberg - SKaMPI   MPI-Benchmark
   Lehrstuhl Informatik fuer Naturwissenschaftler und Ingenieure
   Fakultaet fuer Informatik
   University of Karlsruhe
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

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"

#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_lib.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_loader.hpp"

#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"

#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/offset/SKaMPIClockSync.h"
#include "reprompi_bench/misc.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


typedef struct {
    long n_rep; /* --repetitions */
} reprompi_sk_options_t;


static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void sk_print_sync_parameters(FILE* f) {
    fprintf(f, "#@clocksync=SKaMPI\n");
}

static void synchronize_clocks(void) {
  global_clock = clock_sync->synchronize_all_clocks(MPI_COMM_WORLD, *(local_clock));
}

static double get_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}

void sk_init_module(int argc, char** argv) {
  ClockSyncLoader loader;

  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = loader.instantiate_clock_sync("alg");
  if( clock_sync != NULL ) {
    // now we make sure it's really a SKaMPI clock sync instance
    if( dynamic_cast<SKaMPIClockSync*>(clock_sync) == NULL ) {
      ZF_LOGE("instantiated clock sync is not of type SKaMPIClockSync. aborting..");
      exit(1);
    }
  } else {
    ZF_LOGV("using default SKaMPI clock sync");
    clock_sync = new SKaMPIClockSync(new SKaMPIClockOffsetAlg(10,100));
  }

}

void sk_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}

extern "C" void register_skampi_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*) std::string("SKaMPI").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_SKAMPI;

  sync_mod->init_module = sk_init_module;
  sync_mod->cleanup_module = sk_cleanup_module;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = sk_print_sync_parameters;
}

