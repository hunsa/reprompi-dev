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
#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/utils/keyvalue_store.h"

#include "synchronization.h"
#include "clock_sync_lib.h"

// Implemented synchronization modules
static reprompib_sync_module_t* sync_modules;

static const int HASHTABLE_SIZE=100;

static const sync_type_t clock_sync_options[] = {
        { "HCA", REPROMPI_CLOCKSYNC_HCA},
        { "HCA2", REPROMPI_CLOCKSYNC_HCA2},
        { "HCA3", REPROMPI_CLOCKSYNC_HCA3},
        { "JK", REPROMPI_CLOCKSYNC_JK },
        { "SKaMPI", REPROMPI_CLOCKSYNC_SKAMPI },
#ifdef HAVE_HWLOC
        { "Topo1", REPROMPI_CLOCKSYNC_TOPO1 },
#endif
        { "Topo2", REPROMPI_CLOCKSYNC_TOPO2 },
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
  reprompib_dictionary_t* params_dict;

  params_dict = get_global_param_store();

  //initialize dictionary
  reprompib_init_dictionary(params_dict, HASHTABLE_SIZE);
  // parse extra parameters into the global dictionary
  reprompib_parse_extra_key_value_options(params_dict, argc, argv);

  parse_sync_options(argc, argv, CLOCK_SYNC_ARG, &sync_module_info);
  if (sync_module_info.name == NULL) {
    sync_module_info.name = strdup("None");
  }
  index = get_sync_module_index(sync_module_info.name);

  if (index < 0) {
    char err_msg[160];
    sprintf(err_msg, "Unknown synchronization module \"--clock-sync=%s\"", sync_module_info.name);
    reprompib_print_error_and_exit(err_msg);
  }

  *sync_mod = sync_modules[index];
  sync_mod->init_module(argc, argv);

  cleanup_sync_options(&sync_module_info);
}


void reprompib_register_sync_modules(void) {
  int sync_module_idx;
  sync_modules = calloc(N_CLOCK_SYNC_TYPES, sizeof(reprompib_sync_module_t));

  sync_module_idx = 0;

  register_no_clock_sync_module(&(sync_modules[sync_module_idx++]));
  register_skampi_module(&(sync_modules[sync_module_idx++]));
  register_jk_module(&(sync_modules[sync_module_idx++]));

  register_hca_module(&(sync_modules[sync_module_idx++]));
  register_hca2_module(&(sync_modules[sync_module_idx++]));
  register_hca3_module(&(sync_modules[sync_module_idx++]));
#ifdef HAVE_HWLOC
  register_topo_aware_sync1_module(&(sync_modules[sync_module_idx++]));
#endif
  register_topo_aware_sync2_module(&(sync_modules[sync_module_idx++]));
}

void reprompib_deregister_sync_modules(void) {
  free(sync_modules);
}





