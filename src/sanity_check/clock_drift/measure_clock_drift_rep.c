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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <time.h>
#include <math.h>
#include "mpi.h"

#include <getopt.h>

#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/misc.h"
#include "clock_drift_utils.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


typedef struct opt {
  int npp;    /* --npp number of ping pongs */
  int reps;   /* --reps number of clock sync tests */
  char testname[256];
  double print_procs_ratio;  /* --print-procs-ratio */
} reprompib_drift_test_opts_t;

static const struct option default_long_options[] = {
    { "npp", required_argument, 0, 'n' },
    { "nrep", required_argument, 0, 'r' },
    { "print-procs-ratio", required_argument, 0, 'p' },
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
};

int parse_drift_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv);

void print_help(char* testname) {
    int my_rank;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == root_proc) {

        printf("\nUSAGE: %s [options]\n", testname);

        printf("options:\n");
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "--nrep",
                    "number of repetitions (default: 1)");
        printf("%-40s %-40s\n", "--npp=<int>",
                    "set the number of ping-pong rounds between two processes to measure offset");
        printf("%-40s %-40s\n", "--print-procs-ratio",
        "set the fraction of the total processes to be tested for clock drift. If print-procs-ratio=0, only the last rank and the rank with the largest power of two are tested (default: 0)");

        printf("\nEXAMPLES: mpirun -np 4 %s --npp=2 --nrep=1 --clock-sync=HCA2 --print-procs-ratio=0.1\n", testname);
        printf("\nEXAMPLES: mpirun -np 4 %s --npp=2 --nrep=2 --clock-sync=HCA2 --steps=5 --print-procs-ratio=0.1\n", testname);
        printf("\n\n");
    }
}


void init_parameters(reprompib_drift_test_opts_t* opts_p, char* name) {
  opts_p->npp = 10;
  opts_p->reps = 1;
  opts_p->print_procs_ratio = 0;
  strcpy(opts_p->testname,name);
}


int parse_drift_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv) {
    int c;

    init_parameters(opts_p, argv[0]);

    opterr = 0;

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "h", default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 'r':
            opts_p->reps = atoi(optarg);
            break;
        case 'p': /* fraction of processes for which to measure the drift (normal distribution)
                   if print_procs_ratio==0, print only the largest power of two and the last rank
                   */
            opts_p->print_procs_ratio = atof(optarg);
            break;
        case 'n': /* number of repetitions (pingpongs) */
            opts_p->npp = atol(optarg);
            break;
        case 'h':
            print_help(opts_p->testname);
            break;
        case '?':
            break;
        }
    }

    if (opts_p->print_procs_ratio < 0 || opts_p->print_procs_ratio > 1) {
      reprompib_print_error_and_exit("Invalid process ratio (should be a number between 0 and 1)");
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options

    return 0;
}

void print_initial_settings(int argc, char* argv[], reprompib_drift_test_opts_t opts,
        print_sync_info_t print_sync_info) {
    int my_rank, np;
    FILE * f;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;

        fprintf(f, "#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            fprintf(f, " %s", argv[i]);
        }
        fprintf(f, "\n");
        fprintf(f, "#@nrep=%d\n", opts.reps);
        fprintf(f, "#@npp=%d\n", opts.npp);
        fprintf(f, "#@timerres=%14.9f\n", MPI_Wtick());

        print_time_parameters(f);
        print_sync_info(f);
    }

}

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    reprompib_drift_test_opts_t opts;
    int master_rank;
    FILE* f;
    reprompib_sync_module_t clock_sync;

    double  min_drift;
    double *all_global_times = NULL;

    int ntestprocs;
    int* testprocs_list;
    int index;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    reprompib_register_sync_modules();

    parse_drift_test_options(&opts, argc, argv);

    reprompib_init_sync_module(argc, argv, &clock_sync);
    REPROMPI_init_timer();

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    generate_test_process_list(opts.print_procs_ratio, &testprocs_list, &ntestprocs);

    if (my_rank == master_rank) {
      all_global_times = (double*) calloc(ntestprocs * opts.reps, sizeof(double));
    }

    print_initial_settings(argc, argv, opts, clock_sync.print_sync_info);

    clock_sync.init_sync();

    Number_ping_pongs = opts.npp;

    for (int i = 0; i < opts.reps; i++) {
      clock_sync.sync_clocks();

      if (my_rank == master_rank) {

        // measure once
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // select the process to exchange pingpongs with
          if (p != master_rank) {
            all_global_times[i * ntestprocs + index] = SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p,
                                                                                        &clock_sync);
          }
        }

      } else {
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // make sure the current rank is in the test list
          if (my_rank == p) {
            SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &clock_sync);
          }
        }
      }

    }

    clock_sync.finalize_sync() ;

    f = stdout;
    if (my_rank == master_rank) {

      fprintf(f,"%14s %3s %14s\n", "wait_time_s", "p", "min_diff");

      for(int i=0; i<opts.reps; i++) {
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];
          min_drift = all_global_times[i * ntestprocs + index];
          fprintf(f, "%14.9f %3d %14.9f\n", 0.0f, p, fabs(min_drift));
        }
      }

      free(all_global_times);
    }

    free(testprocs_list);
    clock_sync.cleanup_module();
    reprompib_deregister_sync_modules();
    MPI_Finalize();
    return 0;
}
