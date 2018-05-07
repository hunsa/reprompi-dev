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
#include <getopt.h>
#include "mpi.h"

#include "reprompi_bench/caching/caching.h"
#include "reprompi_bench/misc.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

typedef struct reprompi_caching_mod_params {
  long cache_size_bytes;
} reprompi_caching_mod_params_t;


enum {
  REPROMPI_ARGS_CACHING_CACHE_SIZE
};


static const long DEFAULT_CACHE_SIZE_BYTES = 512 * 1024;
static const char MODULE_NAME[] = "memset";

static char* cache_str = NULL;
static reprompi_caching_mod_params_t opts;


static void parse_caching_module_options(int argc, char **argv, reprompi_caching_mod_params_t* opts_p) {
    int c;

    static const struct option caching_long_options[] = {
            { "cache-size-kb", required_argument, 0, REPROMPI_ARGS_CACHING_CACHE_SIZE },
            { 0, 0, 0, 0 }
    };
    static const char caching_opts_str[] = "";

    opts_p->cache_size_bytes = DEFAULT_CACHE_SIZE_BYTES;

    optind = 1;
    optopt = 0;
    opterr = 0; // ignore invalid options
    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, caching_opts_str, caching_long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case REPROMPI_ARGS_CACHING_CACHE_SIZE:
            opts_p->cache_size_bytes = atol(optarg) * 1024;
            break;
        case '?':
             break;
        }
    }

    // check for errors
    if (opts_p->cache_size_bytes <= 0) {
      reprompib_print_error_and_exit("cache-size-kb error: the cache size must be positive");
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr
}




static void init_module(int argc, char** argv) {
  parse_caching_module_options(argc, argv, &opts);

  cache_str = (char*)malloc(opts.cache_size_bytes);
  ZF_LOGV("Using clear-cache=memset with cache_size_bytes=%ld", opts.cache_size_bytes);
};


static void cleanup_module(void) {
  free(cache_str);
};

static void clear_cache(void) {
  memset(cache_str,'1', opts.cache_size_bytes);
};


static void print_parameters(FILE* f) {
    fprintf(f, "#@clear-cache=%s\n", MODULE_NAME);
}



void register_clear_cache_memset_module(reprompi_caching_module_t *cache_mod) {
  cache_mod->name = strdup(MODULE_NAME);
  cache_mod->type = REPROMPI_CLEAR_CACHE_MEMSET;

  cache_mod->init_module = init_module;
  cache_mod->cleanup_module = cleanup_module;

  cache_mod->clear_cache = clear_cache;

  cache_mod->print_info = print_parameters;
}


