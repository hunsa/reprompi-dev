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
#include "parse_timing_options.h"


typedef struct runtime_type {
    reprompi_timing_method_t type;
    char* name;
} runtime_type_t;

static const runtime_type_t runtime_types[] = {
    { REPROMPI_RUNT_MAX_OVER_LOCAL_RUNTIME, "local" },      // max over the local runtime of all processes for each iteration (default method when using local clocks)
    { REPROMPI_RUNT_GLOBAL_TIMES, "global" },               // max global end_time - min global start_time (when global clocks are enabled)
    //{ REPROMPI_RUNT_MAX_OVER_LOCAL_AVG, "local_avg" }       // OSU method: max over local averages of processes
};
static const int N_RUNTIME_TYPES = sizeof(runtime_types)/sizeof(runtime_type_t);


enum reprompi_timing_getopt_ids {
  REPROMPI_ARGS_RUNTIME_TYPE = 1000   // local or global clocks
};

static const struct option reprompi_timing_long_options[] = {
        {"runtime-type", required_argument, 0, REPROMPI_ARGS_RUNTIME_TYPE},
        { 0, 0, 0, 0 }
};
static const char reprompi_timing_opts_str[] = "v";



const char* reprompib_get_timing_method_name(reprompi_timing_method_t runtime_type) {
   int i;
   for (i=0; i<N_RUNTIME_TYPES; i++) {
     if (runtime_types[i].type == runtime_type) {
       return runtime_types[i].name;
     }
   }
   return NULL;
}


static void parse_runtime_type(char* optarg, reprompi_timing_method_t* timing_method) {
  int i;
  int error = 1;

  for (i=0; i<N_RUNTIME_TYPES; i++) {
    if (strcmp(runtime_types[i].name, optarg) == 0) {
      *timing_method = runtime_types[i].type;
      error = 0;
      break;
    }
  }
  if (error) {
    reprompib_print_error_and_exit("Incorrect run-time type (--runtime-type=<type> (local, global))");
  }
}


void reprompib_parse_timing_options(reprompi_timing_method_t* timing_method, int argc, char** argv) {
    int c;
    opterr = 0;

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_timing_opts_str, reprompi_timing_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case REPROMPI_ARGS_RUNTIME_TYPE: /* runtime computation method */
            parse_runtime_type(optarg, timing_method);
            break;
        case '?':
            break;
        }
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options
}


