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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include "mpi.h"

#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "parse_test_options.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_initial_settings(int argc, char* argv[],
        print_sync_info_t print_sync_info) {
    int my_rank;
    FILE* f;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;
        fprintf(f, "#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            fprintf(f, " %s", argv[i]);
        }
        fprintf(f, "\n");

        print_time_parameters(f);
        print_sync_info(f);
    }

}


int main(int argc, char* argv[]) {
    int my_rank, nprocs;
    reprompib_st_opts_t opts;
    reprompib_sync_module_t clock_sync;
    FILE* f;

//    double *max_runtimes = NULL;
    double *runtimes;
    double start_time;

    MPI_Init(&argc, &argv);

    parse_test_options(&opts, argc, argv);

    reprompib_register_sync_modules();
    reprompib_init_sync_module(argc, argv, &clock_sync);

    REPROMPI_init_timer();

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

//    if (my_rank == OUTPUT_ROOT_PROC) {
//        max_runtimes = (double*) calloc(nprocs * opts.n_rep, sizeof(double));
//    }
    runtimes = (double*) calloc(opts.n_rep, sizeof(double));

    print_initial_settings(argc, argv, clock_sync.print_sync_info);

    clock_sync.init_sync();
    //REPROMPI_init_timer();

    for(int i=0; i<opts.n_rep; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = REPROMPI_get_time();
        clock_sync.sync_clocks();
        runtimes[i] = REPROMPI_get_time() - start_time;
        //printf("%d: time[%d]=%g\n", my_rank, i, runtimes[i]);
    }

    clock_sync.finalize_sync();
    MPI_Reduce(MPI_IN_PLACE, runtimes, opts.n_rep, MPI_DOUBLE,
               MPI_MAX, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        fprintf(f, "  i    runtime\n");
        for(int i=0; i<opts.n_rep; i++) {
            fprintf(f, "%3d %14.9f\n", i, runtimes[i]);
        }
    }

    free(runtimes);

    clock_sync.cleanup_module();
    reprompib_deregister_sync_modules();
    MPI_Finalize();
    return 0;
}
