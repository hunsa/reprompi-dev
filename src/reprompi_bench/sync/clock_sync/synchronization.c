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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/common/sync_module_helpers.h"
#include "synchronization.h"


// Implemented synchronization modules
static reprompib_sync_module_t* sync_modules;

static const sync_type_t clock_sync_options[] = {
        { "HCA", REPROMPI_CLOCKSYNC_HCA},
        { "HCA3", REPROMPI_CLOCKSYNC_HCA3},
        { "JK", REPROMPI_CLOCKSYNC_JK },
        { "SKaMPI", REPROMPI_CLOCKSYNC_SKAMPI },
        { "None", REPROMPI_CLOCKSYNC_NONE }
};
static const int N_CLOCK_SYNC_TYPES = sizeof(clock_sync_options)/sizeof(sync_type_t);
static const char CLOCK_SYNC_ARG[] = "clock-sync";


static int get_sync_module_index(const char* name) {
  int i;
  int sync_type;

  if (name == NULL) {
    return -1;
  }
  sync_type = get_sync_type(N_CLOCK_SYNC_TYPES, clock_sync_options, name);

  if (sync_type < 0) {
    return -1;
  }
  for (i=0; i<N_CLOCK_SYNC_TYPES; i++) {
    if (sync_modules[i].clocksync == sync_type) {
        return i;
    }
  }
  return -1;
}

/**********************************************
 * Initialization/cleanup functions for the specified sync module
 **********************************************/
void reprompib_init_sync_module(int argc, char** argv, reprompib_sync_module_t* sync_mod) {
  sync_module_info_t sync_module_info;
  int index;

  parse_sync_options(argc, argv, CLOCK_SYNC_ARG, &sync_module_info);
  if (sync_module_info.name == NULL) {
    sync_module_info.name = strdup("None");
  }
  index = get_sync_module_index(sync_module_info.name);

  if (index < 0) {
    reprompib_print_error_and_exit("No synchronization module defined for the selected \"--clock-sync\" parameter.");
  }

  *sync_mod = sync_modules[index];
  sync_mod->init_module(argc, argv);

  cleanup_sync_options(&sync_module_info);
}

void reprompib_register_sync_modules(void) {
  sync_modules = calloc(N_CLOCK_SYNC_TYPES, sizeof(reprompib_sync_module_t));

  register_no_clock_sync_module(&(sync_modules[0]));
  register_hca_module(&(sync_modules[1]));
  register_skampi_module(&(sync_modules[2]));
  register_jk_module(&(sync_modules[3]));

  register_hca3_module(&(sync_modules[4]));
}

void reprompib_deregister_sync_modules(void) {
  free(sync_modules);
}




