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
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clock_sync_common.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/JKClockSync.h"
#include "reprompi_bench/misc.h"



static ClockSyncInterface* clock_sync;
static Clock* local_clock;
static Clock* global_clock;


static void jk_print_sync_parameters(FILE* f) {
    fprintf(f, "#@clocksync=JK\n");
}

static void synchronize_clocks(void) {
  global_clock = clock_sync->synchronize_all_clocks();
}

static double get_normalized_time(double local_time) {
  return default_get_normalized_time(local_time, global_clock);
}


void jk_init_module(int argc, char** argv) {
    global_clock = NULL;
    local_clock = new RdtscpClock();
    clock_sync = new JKClockSync<PingpongClockOffsetAlg>(MPI_COMM_WORLD, local_clock);
}


void jk_cleanup_module(void) {
  delete local_clock;
  delete clock_sync;

  if (global_clock != NULL) {
    delete global_clock;
  }
}

extern "C"
void register_jk_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = (char*)std::string("JK").c_str();
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_JK;

  sync_mod->init_module = jk_init_module;
  sync_mod->cleanup_module = jk_cleanup_module;

  sync_mod->init_sync = default_init_synchronization;
  sync_mod->finalize_sync = default_finalize_synchronization;

  sync_mod->sync_clocks = synchronize_clocks;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = jk_print_sync_parameters;
}





