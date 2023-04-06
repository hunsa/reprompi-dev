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

// avoid getsubopt bug
#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"

int main(int argc, char* argv[]) {
    int rank, size;
    int master_rank = 0;
    reprompib_sync_module_t clock_sync;
    int rep = 100;
    int time_reps = 10000;

    MPI_Init(&argc, &argv);

    reprompib_register_sync_modules();

    reprompib_init_sync_module(argc, argv, &clock_sync);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if( argc < 3 ) {
        if( rank == 0 ) {
            printf("usage: %s [rep sync] [rep clock checks]\n", argv[0]);
        }
        exit(1);
    } else {
        rep = atoi(argv[1]);
        time_reps = atoi(argv[2]);
    }

    if( rank == master_rank ) {
        printf("init sync\n");
        fflush(stdout);
    }
    clock_sync.init_sync();

    for(int i=0; i<rep; i++) {
        if( rank == master_rank ) {
            printf("s");
            fflush(stdout);
        }
        clock_sync.sync_clocks();
        for(int j=0; j<time_reps; j++) {
            clock_sync.get_global_time(get_time());
            if( rank == master_rank ) {
                printf(".");
                fflush(stdout);
            }
        }

        if( rank == master_rank ) {
            printf("e");
            fflush(stdout);
        }
    }
    if( rank == master_rank ) {
        printf("\n");
        fflush(stdout);
    }



    if( rank == master_rank ) {
        printf("cleanup sync\n");
        fflush(stdout);
    }
    clock_sync.cleanup_module();
    reprompib_deregister_sync_modules();
    MPI_Finalize();
    return 0;
}
