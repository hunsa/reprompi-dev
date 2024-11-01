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
#include <vector>
#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
//#include "reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_lib.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_loader.hpp"
#include "reprompi_bench/sync/clock_sync/sync_methods/TwoLevelClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/offset/SKaMPIClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockPropagationSync.h"
#include "reprompi_bench/sync/common/sync_module_helpers.h"
#include "reprompi_bench/misc.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void topo_synchronize_clocks(void) {
  if( global_clock != NULL ) {
    delete global_clock;
  }
  global_clock = clock_sync->synchronize_all_clocks(MPI_COMM_WORLD, *(local_clock));
}

static double topo_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}


static void topo_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}



static void topo_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=TwoLevelSync\n");
}


static void topo_init_module(int argc, char** argv) {
  int use_default = 0;
  BaseClockSync *alg1;
  BaseClockSync *alg2;
  ClockSyncLoader loader;

  alg1 = loader.instantiate_clock_sync("topoalg1");
  if( alg1 != NULL ) {
    alg2 = loader.instantiate_clock_sync("topoalg2");
    if( alg2 != NULL ) {
      // now instantiate new two level clock sync
      clock_sync = new TwoLevelClockSync(alg1, alg2);
    } else {
      use_default = 1;
    }
  } else {
    use_default = 1;
  }

  global_clock = NULL;
  local_clock  = initialize_local_clock();

  if( use_default == 1 ) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if( rank == 0 ) {
      ZF_LOGW("!!! using default topo1 clock sync options");
    }

    clock_sync = new TwoLevelClockSync(
      new HCA3ClockSync(new SKaMPIClockOffsetAlg(10,100), 500, false),
      new ClockPropagationSync(ClockPropagationSync::ClockType::CLOCK_LM));
  }

}


extern "C"
void register_topo_aware_sync2_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("Topo2").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_TOPO2;
  sync_mod->init_module = topo_init_module;
  sync_mod->cleanup_module = topo_cleanup_module;
  sync_mod->sync_clocks = topo_synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = topo_normalized_time;
  sync_mod->print_sync_info = topo_print_sync_parameters;
}


