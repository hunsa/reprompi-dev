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
static const int n_sync_modules = 5;
static reprompib_sync_module_t* sync_modules;


/***********
 * Get the name of the sync module from the command line
 *********/
typedef enum reprompi_sync_module_getopt_ids {
  REPROMPI_ARGS_SYNC_TYPE
} reprompi_sync_module_getopt_ids_t;


static const struct option reprompi_sync_module_long_options[] = {
        { "sync", required_argument, 0, REPROMPI_ARGS_SYNC_TYPE },
        { 0, 0, 0, 0 }
};
static const char reprompi_sync_module_opts_str[] = "";


typedef struct sync_module_info {
  char* sync_type;
} sync_module_info_t;

static void parse_sync_options(int argc, char **argv, sync_module_info_t* opts_p) {
    int c;

    opts_p->sync_type = NULL;

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
        case '?':
             break;
        }
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr
}


static int get_sync_module_index(const char* name) {
  int i;

  if (name == NULL) {
    return -1;
  }

  for (i=0; i<n_sync_modules; i++) {
    if (strcmp(name, sync_modules[i].name) == 0) {
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
  index = get_sync_module_index(sync_module_info.sync_type);

  if (index < 0) {
    reprompib_print_error_and_exit("Invalid synchronization module selected (choose between: MPI_Barrier, Dissem_Barrier, HCA, SKaMPI, JK)");
  }

  *sync_mod = sync_modules[index];
  sync_mod->init_module(argc, argv);
}


void reprompib_register_sync_modules(void) {
  sync_modules = calloc(n_sync_modules, sizeof(reprompib_sync_module_t));

  register_hca_module(&(sync_modules[0]));
  register_skampi_module(&(sync_modules[1]));
  register_mpibarrier_module(&(sync_modules[2]));
  register_dissem_barrier_module(&(sync_modules[3]));
  register_jk_module(&(sync_modules[4]));
}

void reprompib_deregister_sync_modules(void) {
  free(sync_modules);
}





