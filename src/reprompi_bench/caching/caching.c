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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include "reprompi_bench/caching/caching.h"
#include "reprompi_bench/misc.h"

// Implemented process synchronization modules
static reprompi_caching_module_t* caching_modules;

typedef struct reprompi_caching_strategy {
  char* name;
  reprompi_caching_type_t type;
}reprompi_caching_strategy_t;


static char DEFAULT_CACHING_MOD_NAME[] = "None";
static const reprompi_caching_strategy_t caching_options[] = {
    { DEFAULT_CACHING_MOD_NAME, REPROMPI_CLEAR_CACHE_NONE},
    { "memset", REPROMPI_CLEAR_CACHE_MEMSET }
};
static const int N_CACHING_MODULES = sizeof(caching_options)/sizeof(reprompi_caching_strategy_t);
static const char CACHING_ARG[] = "clear-cache";

enum{
  REPROMPI_ARGS_CACHING_TYPE = 1
};



static int get_caching_module_index(const char* name, const int n_modules, const reprompi_caching_module_t* caching_modules) {
  int i;
  int type;

  if (name == NULL) {
    return -1;
  }
  type = -1;
  for (i=0; i<N_CACHING_MODULES; i++) {
    if (strcmp(name, caching_options[i].name) == 0) {
      type = caching_options[i].type;
      break;
    }
  }

  for (i=0; i<n_modules; i++) {
    if (type == caching_modules[i].type) {
      return i;
    }
  }
  return -1;
}



void parse_caching_options(int argc, char **argv, const char* argument_name,
    char** caching_type) {
  int c;
  struct option reprompi_caching_module_long_options[] = {
      { argument_name, required_argument, 0, REPROMPI_ARGS_CACHING_TYPE },
      { 0, 0, 0, 0 }
  };
  const char reprompi_caching_module_opts_str[] = "";

  *caching_type = NULL;

  optind = 1;
  optopt = 0;
  opterr = 0; // ignore invalid options
  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, reprompi_caching_module_opts_str, reprompi_caching_module_long_options,
        &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case REPROMPI_ARGS_CACHING_TYPE: /* caching module */
      *caching_type = strdup(optarg);
      break;
    case '?':
      break;
    }
  }
  optind = 1; // reset optind to enable option re-parsing
  opterr = 1; // reset opterr
}



/**********************************************
 * Initialization/cleanup functions for the specified sync module
 **********************************************/

void reprompib_init_caching_module(int argc, char** argv, reprompi_caching_module_t* caching_module) {
  int index;
  char* caching_module_name = NULL;

  parse_caching_options(argc, argv, CACHING_ARG, &caching_module_name);
  if (caching_module_name == NULL) {  // default strategy - don't clear caches
    caching_module_name = strdup(DEFAULT_CACHING_MOD_NAME);
  }
  index = get_caching_module_index(caching_module_name, N_CACHING_MODULES, caching_modules);

  if (index < 0) {
    char err_msg[160];
    sprintf(err_msg, "Unknown caching module \"--%s=%s\"", CACHING_ARG, caching_module_name);
    reprompib_print_error_and_exit(err_msg);
  }

  *caching_module = caching_modules[index];
  caching_module->init_module(argc, argv);

  if (caching_module_name != NULL) {
    free(caching_module_name);
  }
}


void reprompib_register_caching_modules(void) {
  int i;
  caching_modules = calloc(N_CACHING_MODULES, sizeof(reprompi_caching_module_t));

  i = 0;
  register_warm_cache_module(&(caching_modules[i++]));
  register_clear_cache_memset_module(&(caching_modules[i++]));
}

void reprompib_deregister_caching_modules(void) {
  free(caching_modules);
}





