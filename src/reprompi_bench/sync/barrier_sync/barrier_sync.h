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

#ifndef REPROMPIB_SYNCHRONIZATION_BARRIER_SYNC_H_
#define REPROMPIB_SYNCHRONIZATION_BARRIER_SYNC_H_

#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

#include "reprompi_bench/sync/synchronization.h"

void dissemination_barrier(void);

double barrier_get_normalized_time(double local_time);
int* barrier_get_errorcodes(void);
void barrier_init_module(int argc, char** argv);
void barrier_init_synchronization(const reprompib_sync_params_t* sync_params);

void empty(void);



void mpibarrier_start_synchronization(void);
void dissem_barrier_start_synchronization(void);


#endif /* REPROMPIB_SYNCHRONIZATION_BARRIER_SYNC_H_ */
