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
#include "option_parser_helpers.h"
#include "parse_options.h"

static const int OUTPUT_ROOT_PROC = 0;


enum reprompi_summary_opts {
    MASK_PRINT_MEAN = 0x01,
    MASK_PRINT_MEDIAN = 0x02,
    MASK_PRINT_MIN = 0x04,
    MASK_PRINT_MAX = 0x08
};
static const int N_SUMMARY_METHODS = 4;

static char* const summary_opts[] = { "mean", "median", "min", "max", NULL};
static summary_method_info_t summary_methods[] = {
    { MASK_PRINT_MEAN, "mean" },
    { MASK_PRINT_MEDIAN, "median" },
    { MASK_PRINT_MIN, "min" },
    { MASK_PRINT_MAX, "max" }
};

typedef struct runtime_type {
    reprompi_timing_method_t type;
    char* name;
} runtime_type_t;

static const runtime_type_t runtime_types[] = {
    { REPROMPI_RUNT_MAX_OVER_LOCAL_RUNTIME, "local" },      // max over the local runtime of all processes for each iteration (default method when using local clocks)
    { REPROMPI_RUNT_GLOBAL_TIMES, "global" },               // max global end_time - min global start_time (when global clocks are enabled)
    { REPROMPI_RUNT_MAX_OVER_LOCAL_AVG, "local_avg" }       // OSU method: max over local averages of processes
};
static const int N_RUNTIME_TYPES = 3;


enum reprompi_common_getopt_ids {
  REPROMPI_ARGS_VERBOSE = 'v',
  REPROMPI_ARGS_NREPS = 500,
  REPROMPI_ARGS_SUMMARY,
  REPROMPI_ARGS_RUNTIME_TYPE    // local or global clocks
};

static const struct option reprompi_default_long_options[] = {
        {"verbose", optional_argument, 0, REPROMPI_ARGS_VERBOSE},
        { "nrep", required_argument, 0, REPROMPI_ARGS_NREPS },
        {"summary", optional_argument, 0, REPROMPI_ARGS_SUMMARY},
        {"runtime-type", required_argument, 0, REPROMPI_ARGS_RUNTIME_TYPE},

        { 0, 0, 0, 0 }
};
static const char reprompi_default_opts_str[] = "v";



const char* reprompib_get_runtime_type_name(int runtime_type) {
   int i;
   for (i=0; i<N_RUNTIME_TYPES; i++) {
     if (runtime_types[i].type == runtime_type) {
       return runtime_types[i].name;
     }
   }
   return NULL;
}

summary_method_info_t* reprompib_get_summary_method(int index) {
   if (index <0 || index >= N_SUMMARY_METHODS) {
     reprompib_print_error_and_exit("Incorrect index for the summary methods (it has to be an integer between 0 and 3)");
   }

   return &(summary_methods[index]);
}

int reprompib_get_number_summary_methods(void) {
  return N_SUMMARY_METHODS;
}


static void init_parameters(reprompib_options_t* opts_p) {
    opts_p->verbose = 0;
    opts_p->n_rep = 0;
    opts_p->print_summary_methods = 0;
    opts_p->runtime_type = -1;
}

void reprompib_free_parameters(reprompib_options_t* opts_p) {
}


static void parse_summary_list(char* subopts, reprompib_options_t* opts_p) {
    char * value;
    int index;

    if (subopts != NULL) {
        while (*subopts != '\0') {
            index = getsubopt(&subopts, summary_opts, &value);

            if (index >=0 && index < reprompib_get_number_summary_methods()) {
                opts_p->print_summary_methods |= reprompib_get_summary_method(index)->mask;
            }
            else {
              reprompib_print_error_and_exit("Invalid list of summary methods (--summary=<list of comma-separated methods> [min, max, mean, median])");
            }
        }
    }
    if (opts_p->print_summary_methods == 0) {  // no method specified - use all of them
        for (index=0; index < reprompib_get_number_summary_methods(); index++) {
          opts_p->print_summary_methods |= reprompib_get_summary_method(index)->mask;
        }
    }
}

static void parse_runtime_type(char* optarg, reprompib_options_t* opts_p) {
  int i;
  int error = 1;

  for (i=0; i<N_RUNTIME_TYPES; i++) {
    if (strcmp(runtime_types[i].name, optarg) == 0) {
      opts_p->runtime_type = runtime_types[i].type;
      error = 0;
      break;
    }
  }
  if (error) {
    reprompib_print_error_and_exit("Incorrect run-time type (--runtime-type=<type> (local, global, or local_avg))");
  }
}


void reprompib_parse_options(reprompib_options_t* opts_p, int argc, char** argv) {
    int c, err;
    long nreps;

    init_parameters(opts_p);
    opterr = 0;

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_default_opts_str, reprompi_default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {

        case REPROMPI_ARGS_NREPS: /* total number of (correct) repetitions */
            err = reprompib_str_to_long(optarg, &nreps);
            if (err || nreps <= 0) {
              reprompib_print_error_and_exit("Nreps value is negative or not correctly specified");
            }
            opts_p->n_rep = nreps;
            break;

        case REPROMPI_ARGS_SUMMARY: /* list of summary options */
            parse_summary_list(optarg, opts_p);
            break;

        case REPROMPI_ARGS_VERBOSE: /* verbose flag */
            opts_p->verbose = 1;
            break;

        case REPROMPI_ARGS_RUNTIME_TYPE: /* runtime computation method */
            parse_runtime_type(optarg, opts_p);
            break;
        case '?':
            break;
        }
    }


//    if (opts_p->n_rep < 0) {
//      reprompib_print_error_and_exit("Nreps value is negative or not correctly specified");
///    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options
}


void reprompib_print_benchmark_help(void) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nUSAGE: mpibenchmark [options]\n");
        printf("options:\n");
    }

    reprompib_print_common_help();

    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nSpecific options for the benchmark execution:\n");
        printf("%-40s %-40s\n", "--nrep=<nrep>",
                "set number of experiment repetitions");
        printf("%-40s %-40s\n %50s%s\n", "--summary=<args>",
                "list of comma-separated data summarizing methods (mean, median, min, max)", "",
                "e.g., --summary=mean,max");

        printf("\nEXAMPLES: mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast --msizes-list=8,512,1024 --nrep=5 --summary=mean,max,min\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast --msizes-list=8,512,1024 --nrep=5\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msize-interval=min=1,max=8,step=1 --nrep=5\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msizes-list=1024 --nrep=5 --fitpoints=10 --exchanges=20\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msizes-list=1024 --nrep=5 --params=p1:1,p2:aaa,p3:34\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --calls-list=Sendrecv --msizes-list=10 --pingpong-ranks=0,3 --nrep=5 --summary \n");

        printf("\n\n");
    }
}

