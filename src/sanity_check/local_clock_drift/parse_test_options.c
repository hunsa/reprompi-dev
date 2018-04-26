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
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "mpi.h"

#include <sanity_check/clock_drift/parse_drift_test_options.h>

static const struct option default_long_options[] = {
        { "nrep", required_argument, 0, 'n' },
        { "steps", required_argument, 0, 's' },
        { "rtt-nrep", required_argument, 0, 'r' },
        { "print-procs-ratio", required_argument, 0, 'p' },
        { "print-procs-allpingpongs", required_argument, 0, 'a' },
        { "help", no_argument, 0, 'h' },
        { 0, 0, 0, 0 }
};



void reprompib_print_error_and_exit(const char* error_str) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "\nERROR: %s\n\n", error_str);
  }
  MPI_Finalize();
  exit(1);
}




void print_help(char* testname) {
    int my_rank;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == root_proc) {

        if (strstr(testname, "measure_clock_drift") != 0) {
            printf("\nUSAGE: %s [options] [steps]\n", testname);
        }
        else {
            printf("\nUSAGE: %s [options]\n", testname);
        }

        printf("options:\n");
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "--nrep=<nrep>",
                    "set number of ping-pong rounds between two processes to measure offset");
        printf("%-40s %-40s\n", "<steps>",
                    "set number of 1s steps to wait after sync (default: 0)");
        printf(
            "\nEXAMPLES: mpirun -np 4 %s --nrep=10 5\n", testname);
        printf("\n\n");
    }
}


void init_parameters(reprompib_drift_test_opts_t* opts_p, char* name) {
    opts_p->n_rep = 0;
    opts_p->steps = 0;

    opts_p->rtt_pingpongs_nrep = 100;
    opts_p->print_procs_ratio = 0;
    opts_p->print_procs_allpingpongs = 1;
    strcpy(opts_p->testname,name);
}


int parse_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv) {
    int c;
    int printhelp = 0;

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
        case 'n': /* number of repetitions (pingpongs) */
            opts_p->n_rep = atol(optarg);
            break;
        case 'r': /* number of pingpongs performed to estimate the RTT between two processes */
            opts_p->rtt_pingpongs_nrep = atol(optarg);
            break;
        case 's': /* number of 1s steps after which to measure the clock drift */
            opts_p->steps = atoi(optarg);
            break;
        case 'p': /* percentage of processes for which to measure the drift (normal distribution)
                   if print_procs_ratio==0, print only the largest power of two and the last rank
                   */
            opts_p->print_procs_ratio = atoi(optarg);
            break;
        case 'a': /* print all pingpong results or only the min clock drift */
            opts_p->print_procs_allpingpongs = atoi(optarg);
            break;
        case 'h':
            print_help(opts_p->testname);
            printhelp = 1;
            break;
        case '?':
            break;
        }
    }

    if (opts_p->n_rep <= 0) {
      reprompib_print_error_and_exit("Invalid number of repetitions (should be positive)");
    }
    if (opts_p->rtt_pingpongs_nrep <= 0) {
      reprompib_print_error_and_exit("Invalid number of repetitions for the RTT (should be positive)");
    }
    if (opts_p->steps < 0) {
      reprompib_print_error_and_exit("Invalid number of steps (should be >=0)");
    }
    if (opts_p->print_procs_ratio < 0) {
      reprompib_print_error_and_exit("Invalid process ratio (should be >=0)");
    }
    if (opts_p->print_procs_allpingpongs <= 0) {
      reprompib_print_error_and_exit("Invalid process ratio (should be positive)");
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options

    return 0;
}

