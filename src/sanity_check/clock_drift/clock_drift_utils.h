//
// Created by Sascha on 5/11/23.
//

#ifndef REPROMPI_CLOCK_DRIFT_UTILS_H
#define REPROMPI_CLOCK_DRIFT_UTILS_H

#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/synchronization.h"

double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank, reprompib_sync_module_t *clock_sync);
void generate_test_process_list(double process_ratio, int **testprocs_list_p, int* ntestprocs);

extern int Minimum_ping_pongs;
extern int Number_ping_pongs;
extern const int OUTPUT_ROOT_PROC;

#endif //REPROMPI_CLOCK_DRIFT_UTILS_H
