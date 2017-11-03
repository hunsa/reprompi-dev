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

#ifndef REPROMPIB_SYNCHRONIZATION_H_
#define REPROMPIB_SYNCHRONIZATION_H_

#include "reprompi_bench/sync/sync_info.h"

typedef enum {
  REPROMPI_CLOCKSYNC_HCA = 0,
  REPROMPI_CLOCKSYNC_HCA3,
  REPROMPI_CLOCKSYNC_JK,
  REPROMPI_CLOCKSYNC_SKAMPI,
  REPROMPI_CLOCKSYNC_NONE
} reprompi_clocksync_t;

typedef enum {
  REPROMPI_PROCSYNC_WIN = 0,
  REPROMPI_PROCSYNC_MPIBARRIER,
  REPROMPI_PROCSYNC_DISSEMBARRIER
} reprompi_procsync_t;

// parameters needed to initialize a synchronization round
typedef struct reprompib_sync_params {
    long nrep;
}reprompib_sync_params_t;



typedef int* (*sync_errorcodes_t)(void);
typedef double (*sync_normtime_t)(double local_time);
typedef void (*print_sync_info_t)(FILE* f);


typedef struct reprompib_sync_module{
    void (*init_module)(int argc, char** argv);
    void (*cleanup_module)(void);

    void (*init_sync)(const reprompib_sync_params_t* init_params);
    void (*finalize_sync)(void);

    void (*sync_clocks)(void);
    void (*init_sync_round)(void);

    void (*start_sync)(void);
    void (*stop_sync)(void);

    sync_errorcodes_t get_errorcodes;
    sync_normtime_t get_global_time;
    print_sync_info_t print_sync_info;

    char* name;
    // a module is uniquely identified by the clock sync. method and the process synchronization method
    reprompi_clocksync_t clocksync;
    reprompi_procsync_t procsync;
} reprompib_sync_module_t;


void reprompib_register_sync_modules(void);
void reprompib_deregister_sync_modules(void);

void reprompib_init_sync_module(int argc, char** argv, reprompib_sync_module_t* sync_mod);
void reprompib_cleanup_sync_module(reprompib_sync_module_t* sync_mod);

void register_hca_module(reprompib_sync_module_t *sync_mod);
void register_jk_module(reprompib_sync_module_t *sync_mod);
void register_skampi_module(reprompib_sync_module_t *sync_mod);
void register_mpibarrier_module(reprompib_sync_module_t *sync_mod);
void register_dissem_barrier_module(reprompib_sync_module_t *sync_mod);

void register_hca_mpibarrier_module(reprompib_sync_module_t *sync_mod);
void register_hca_dissembarrier_module(reprompib_sync_module_t *sync_mod);

void register_hca3_module(reprompib_sync_module_t *sync_mod);

#endif /* REPROMPIB_SYNCHRONIZATION_H_ */
