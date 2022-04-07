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

#ifndef REPROMPIB_PROCESS_SYNCHRONIZATION_H_
#define REPROMPIB_PROCESS_SYNCHRONIZATION_H_

#include "reprompi_bench/sync/clock_sync/synchronization.h"

enum {
    REPROMPI_INVALID_MEASUREMENT = 1,
    REPROMPI_CORRECT_MEASUREMENT = 0,
    REPROMPI_OUT_OF_TIME_VALID   = 2,
    REPROMPI_OUT_OF_TIME_INVALID = 3
};

typedef enum {
  REPROMPI_PROCSYNC_WIN = 0,
  REPROMPI_PROCSYNC_MPIBARRIER,
  REPROMPI_PROCSYNC_DISSEMBARRIER,
  REPROMPI_PROCSYNC_DOUBLE_MPIBARRIER,
  REPROMPI_PROCSYNC_ROUNDTIMESYNC,
  REPROMPI_PROCSYNC_NONE,
} reprompi_procsync_t;

// parameters needed to initialize a synchronization round
typedef struct reprompib_sync_params {
  long nrep;
  int count;
} reprompib_sync_params_t;



typedef int* (*sync_errorcodes_t)(void);

typedef struct reprompib_proc_sync_module{
    void (*init_module)(int argc, char** argv, reprompib_sync_module_t* clock_sync);
    void (*cleanup_module)(void);

    void (*init_sync)(const reprompib_sync_params_t* init_params);
    void (*finalize_sync)(void);

    void (*init_sync_round)(void);
    void (*start_sync)(MPI_Comm comm);
    int (*stop_sync)(MPI_Comm comm);

    print_sync_info_t print_sync_info;
    sync_errorcodes_t get_errorcodes;

    char* name;
    // a module is uniquely identified by the process synchronization method
    reprompi_procsync_t procsync;
} reprompib_proc_sync_module_t;


void reprompib_register_proc_sync_modules(void);
void reprompib_deregister_proc_sync_modules(void);

void reprompib_init_proc_sync_module(int argc, char** argv, reprompib_sync_module_t* clock_sync, reprompib_proc_sync_module_t* sync_mod);
void reprompib_cleanup_proc_sync_module(reprompib_proc_sync_module_t* sync_mod);

void register_window_module(reprompib_proc_sync_module_t *sync_mod);
void register_dissem_barrier_module(reprompib_proc_sync_module_t *sync_mod);
void register_mpibarrier_module(reprompib_proc_sync_module_t *sync_mod);
void register_roundtimesync_module(reprompib_proc_sync_module_t *sync_mod);
void register_procsync_none_module(reprompib_proc_sync_module_t *sync_mod);
void register_double_mpibarrier_module(reprompib_proc_sync_module_t *sync_mod);

#endif /* REPROMPIB_SYNCHRONIZATION_H_ */
