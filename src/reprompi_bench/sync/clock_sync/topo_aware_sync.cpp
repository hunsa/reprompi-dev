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

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_lib.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HierarchicalClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/SKaMPIClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA2ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCA3ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockPropagationSync.h"
#include "reprompi_bench/misc.h"

static ClockSync* clock_sync;
static Clock* local_clock;
static GlobalClock* global_clock;


static void topo_synchronize_clocks(void) {
  global_clock = clock_sync->synchronize_all_clocks(MPI_COMM_WORLD, *(local_clock));

  /*
  GlobalClockLM* lmclock = NULL;
  if (global_clock != NULL) {
    lmclock = dynamic_cast<GlobalClockLM*>(global_clock);
    if (lmclock != NULL) {
      printf("clock: %20.9f   %20.9f\n", lmclock->get_slope(), lmclock->get_intercept());
    }
  } */

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



static void topo1_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=Topo1\n");
//  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
//  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
}

static void topo2_print_sync_parameters(FILE* f)
{
  fprintf (f, "#@clocksync=Topo2\n");
//  fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
//  fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
}

static void topo1_init_module(int argc, char** argv) {
  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = new HierarchicalClockSync(
    //new HCA3ClockSync(new SKaMPIClockOffsetAlg(), 300, 100),
    new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100),
    //new ClockPropagationSync(),
    new HCA3ClockSync(new SKaMPIClockOffsetAlg(), 300, 100),
    //new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100),
    new ClockPropagationSync());
    //new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100));
}


static void topo2_init_module(int argc, char** argv) {
  global_clock = NULL;
  local_clock = initialize_local_clock();

  clock_sync = new HierarchicalClockSync(
      new HCA3ClockSync(new PingpongClockOffsetAlg(), 1000, 100),
      new JKClockSync(new PingpongClockOffsetAlg(), 1000, 100),
      new JKClockSync(new PingpongClockOffsetAlg(), 1000, 100));
}


extern "C"
void register_topo_aware_sync1_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("Topo1").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_TOPO1;
  sync_mod->init_module = topo1_init_module;
  sync_mod->cleanup_module = topo_cleanup_module;
  sync_mod->sync_clocks = topo_synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = topo_normalized_time;
  sync_mod->print_sync_info = topo1_print_sync_parameters;
}


extern "C"
void register_topo_aware_sync2_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("Topo2").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_TOPO2;
  sync_mod->init_module = topo2_init_module;
  sync_mod->cleanup_module = topo_cleanup_module;
  sync_mod->sync_clocks = topo_synchronize_clocks;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->get_global_time = topo_normalized_time;
  sync_mod->print_sync_info = topo2_print_sync_parameters;
}
