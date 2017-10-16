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
#include <mpi.h>

//#include "time_measurement.h"
#include "reprompi_bench/misc.h"
#include "synchronization.h"


// Implemented synchronization modules
static const int n_sync_modules = 7;
static reprompib_sync_module_t* sync_modules;

typedef struct sync_type {
  char* name;
  int type;
}sync_type_t;
static const sync_type_t proc_sync_options[] = {
        { "window", REPROMPI_SYNCTYPE_WIN},
        { "MPI_Barrier", REPROMPI_SYNCTYPE_MPIBARRIER },
        { "dissem_barrier", REPROMPI_SYNCTYPE_DISSEMBARRIER }
};
static const int N_PROC_SYNC_TYPES = sizeof(proc_sync_options)/sizeof(sync_type_t);


static const sync_type_t clock_sync_options[] = {
        { "HCA", REPROMPI_CLOCKSYNC_HCA},
        { "JK", REPROMPI_CLOCKSYNC_JK },
        { "SKaMPI", REPROMPI_CLOCKSYNC_SKAMPI },
        { "None", REPROMPI_CLOCKSYNC_NONE }
};
static const int N_CLOCK_SYNC_TYPES = sizeof(clock_sync_options)/sizeof(sync_type_t);

/***********
 * Get the name of the sync module from the command line
 *********/
typedef enum reprompi_sync_module_getopt_ids {
  REPROMPI_ARGS_SYNC_TYPE,            // clock sync: HCA, SKaMPI etc. (default: None)
  REPROMPI_ARGS_PROC_SYNC_TYPE        // process sync: window, MPI_Barrier, Dissem_Barrier
} reprompi_sync_module_getopt_ids_t;


static const struct option reprompi_sync_module_long_options[] = {
        { "clock-sync", required_argument, 0, REPROMPI_ARGS_SYNC_TYPE },
        { "proc-sync", required_argument, 0, REPROMPI_ARGS_PROC_SYNC_TYPE },
        { 0, 0, 0, 0 }
};
static const char reprompi_sync_module_opts_str[] = "";


typedef struct sync_module_info {
  char* sync_type;
  char* proc_sync;
} sync_module_info_t;

static void parse_sync_options(int argc, char **argv, sync_module_info_t* opts_p) {
    int c;

    opts_p->sync_type = NULL;
    opts_p->proc_sync = NULL;

    optind = 1;
    optopt = 0;
    opterr = 0; // ignore invalid options
    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_sync_module_opts_str, reprompi_sync_module_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case REPROMPI_ARGS_SYNC_TYPE: /* synchronization module */
            opts_p->sync_type = strdup(optarg);
            break;
        case REPROMPI_ARGS_PROC_SYNC_TYPE: /* synchronization module */
            opts_p->proc_sync = strdup(optarg);
            break;
        case '?':
             break;
        }
    }

    if (opts_p->sync_type == NULL) {
      opts_p->sync_type = strdup("None");
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr
}

static void cleanup_sync_options(sync_module_info_t* opts_p) {
  if (opts_p != NULL && opts_p->sync_type != NULL) {
    free(opts_p->sync_type);
  }
  if (opts_p != NULL && opts_p->proc_sync != NULL) {
    free(opts_p->proc_sync);
  }
}


static int get_sync_type(const int n_types, const sync_type_t* type_list, const char* name) {
  int i;

  if (name == NULL) {
    return -1;
  }

  for (i=0; i<n_types; i++) {
    if (strcmp(name, type_list[i].name) == 0) {
        return i;
    }
  }

  return -1;
}

static int get_sync_module_index(const sync_module_info_t* sync_module_info) {
  int i;
  int clock_sync_type, proc_sync_type;

  if (sync_module_info == NULL) {
    return -1;
  }

  clock_sync_type = get_sync_type(N_CLOCK_SYNC_TYPES, clock_sync_options, sync_module_info->sync_type);
  proc_sync_type = get_sync_type(N_PROC_SYNC_TYPES, proc_sync_options, sync_module_info->proc_sync);

  if (clock_sync_type < 0 || proc_sync_type < 0) {
    return -1;
  }

  for (i=0; i<n_sync_modules; i++) {
    if (sync_modules[i].clocksync == clock_sync_type && sync_modules[i].sync_type == proc_sync_type) {
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

  parse_sync_options(argc, argv, &sync_module_info);
  index = get_sync_module_index(&sync_module_info);

  if (index < 0) {
    reprompib_print_error_and_exit("No synchronization module defined for the selected combination of --clock-sync and --proc-sync paramters.");
  }

  *sync_mod = sync_modules[index];
  sync_mod->init_module(argc, argv);

  cleanup_sync_options(&sync_module_info);
}


void reprompib_register_sync_modules(void) {
  sync_modules = calloc(n_sync_modules, sizeof(reprompib_sync_module_t));

  register_hca_module(&(sync_modules[0]));
  register_skampi_module(&(sync_modules[1]));
  register_mpibarrier_module(&(sync_modules[2]));
  register_dissem_barrier_module(&(sync_modules[3]));
  register_jk_module(&(sync_modules[4]));

  register_hca_mpibarrier_module(&(sync_modules[5]));
  register_hca_dissembarrier_module(&(sync_modules[6]));
}

void reprompib_deregister_sync_modules(void) {
  free(sync_modules);
}





