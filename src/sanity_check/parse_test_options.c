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

#include "reprompi_bench/misc.h"
#include "parse_test_options.h"

static const struct option default_long_options[] = {
        { "nrep", required_argument, 0, 'r' },
        { "help", no_argument, 0, 'h' },
        { 0, 0, 0, 0 }
};


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
        printf("%-40s %-40s\n", "--window-size=<win>",
                "window size in microseconds for Window-based synchronization");
        printf("\nSpecific Options for the linear model of the clock skew:\n");
        printf("%-40s %-40s\n", "--fitpoints=<nfit>",
                "number of fitpoints (default: 20)");
        printf("%-40s %-40s\n", "--exchanges=<nexc>",
                "number of exchanges (default: 10)");
        if (strstr(testname, "measure_clock_drift") != 0) {

            printf("%-40s %-40s\n", "--nrep=<nrep>",
                    "set number of ping-pong rounds between two processes to measure offset");
            printf("%-40s %-40s\n", "<steps>",
                    "set number of 1s steps to wait after sync (default: 0)");

            printf(
                    "\nEXAMPLES: mpirun -np 4 %s --nrep=5 --window-size=100 --fitpoints=10 --exchanges=20 5\n", testname);
            printf(
                    "\n          mpirun -np 4 %s --nrep=5 --window-size=100 \n", testname);
        }
        else {
            printf(
                    "\nEXAMPLES: mpirun -np 4 %s --nrep=5 --window-size=100 --fitpoints=10 --exchanges=20\n", testname);
            printf(
                    "\n          mpirun -np 4 %s --nrep=5 --window-size=100 \n", testname);

        }

        printf("\n\n");
    }
}


void init_parameters(reprompib_st_opts_t* opts_p, char* name) {
    opts_p->n_rep = 0;
    opts_p->steps = 0;
    strcpy(opts_p->testname,name);
}


int parse_test_options(reprompib_st_opts_t* opts_p, int argc, char **argv) {
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
        case 'r': /* total number of (correct) repetitions */
            opts_p->n_rep = atol(optarg);
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

    if (optind < argc)
    {
        opts_p->steps = atoi(argv[optind++]);
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options

    return 0;
}

