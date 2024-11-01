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

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/common/sync_module_helpers.h"
#include "process_synchronization.h"


// Implemented process synchronization modules
static reprompib_proc_sync_module_t* sync_modules;

static const sync_type_t proc_sync_options[] = {
        { "window", REPROMPI_PROCSYNC_WIN},
        { "MPI_Barrier", REPROMPI_PROCSYNC_MPIBARRIER },
        { "dissem_barrier", REPROMPI_PROCSYNC_DISSEMBARRIER },
        { "roundtime", REPROMPI_PROCSYNC_ROUNDTIMESYNC },
        { "None", REPROMPI_PROCSYNC_NONE },
        { "Double_MPI_Barrier", REPROMPI_PROCSYNC_DOUBLE_MPIBARRIER }
};
static const int N_PROC_SYNC_TYPES = sizeof(proc_sync_options)/sizeof(sync_type_t);
static const char PROC_SYNC_ARG[] = "proc-sync";

static int get_sync_module_index(const char* name) {
  int i;
  int proc_sync_type;

  if (name == NULL) {
    return -1;
  }
  proc_sync_type = get_sync_type(N_PROC_SYNC_TYPES, proc_sync_options, name);

  if (proc_sync_type < 0) {
    return -1;
  }

  for (i=0; i<N_PROC_SYNC_TYPES; i++) {
    if (sync_modules[i].procsync == proc_sync_type) {
        return i;
    }
  }
  return -1;
}

static void print_available_modules() {
  int i, rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if( rank == 0 ) {
    printf("\nAvailable options for %s are: ", PROC_SYNC_ARG);
    for(i=0; i<N_PROC_SYNC_TYPES; i++) {
      if( i > 0 ) printf(", ");
      printf("%s", proc_sync_options[i].name);
    }
    printf("\n");
  }
}

/**********************************************
 * Initialization/cleanup functions for the specified sync module
 **********************************************/
void reprompib_init_proc_sync_module(int argc, char** argv, mpits_clocksync_t* clock_sync,
    reprompib_proc_sync_module_t* sync_mod) {
  sync_module_info_t sync_module_info;
  int index;

  parse_sync_options(argc, argv, PROC_SYNC_ARG, &sync_module_info);
  if (sync_module_info.name == NULL) {
    sync_module_info.name = strdup("MPI_Barrier");
  }
  index = get_sync_module_index(sync_module_info.name);

  if (index < 0) {
    char err[256];
    char *name = sync_module_info.name;
    if( name == NULL ) {
      name = "UNKNOWN";
    }
    sprintf(err, "No process synchronization module defined for the selected --proc-sync parameter \"%s\".", name);
    print_available_modules();
    reprompib_print_error_and_exit(err);
  }

  *sync_mod = sync_modules[index];
  sync_mod->init_module(argc, argv, clock_sync);

  cleanup_sync_options(&sync_module_info);
}


void reprompib_register_proc_sync_modules(void) {
  sync_modules = calloc(N_PROC_SYNC_TYPES, sizeof(reprompib_proc_sync_module_t));

  register_mpibarrier_module(&(sync_modules[0]));
  register_dissem_barrier_module(&(sync_modules[1]));
  register_window_module(&(sync_modules[2]));
  register_roundtimesync_module(&(sync_modules[3]));
  register_procsync_none_module(&(sync_modules[4]));
  register_double_mpibarrier_module(&(sync_modules[5]));
}

void reprompib_deregister_proc_sync_modules(void) {
  free(sync_modules);
}





