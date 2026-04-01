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
#include "mpits.h"

int main(int argc, char* argv[]) {
    int rank, size;
    int master_rank = 0;
    mpits_clocksync_t cs;
    int rep = 100;
    int time_reps = 10000;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if( argc < 3 ) {
        if( rank == 0 ) {
            printf("usage: %s [rep sync] [rep clock checks]\n", argv[0]);
        }
        MPI_Finalize();
        exit(0);
    } else {
        rep = atoi(argv[1]);
        time_reps = atoi(argv[2]);
        //printf("%d: rep=%d time_reps=%d\n", rank, rep, time_reps);
    }

    MPITS_Init(MPI_COMM_WORLD, &cs);
    MPITS_Clocksync_init(&cs);

    if( rank == master_rank ) {
        printf("init sync\n");
        fflush(stdout);
    }

    for(int i=0; i<rep; i++) {
        if( rank == master_rank ) {
            printf("s");
            fflush(stdout);
        }
        MPITS_Clocksync_sync(&cs);
        for(int j=0; j<time_reps; j++) {
            MPITS_Clocksync_get_time(&cs);
//            if( rank == master_rank ) {
//                printf(".");
//                fflush(stdout);
//            }
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

    MPITS_Clocksync_finalize(&cs);
    MPI_Finalize();
    return 0;
}
