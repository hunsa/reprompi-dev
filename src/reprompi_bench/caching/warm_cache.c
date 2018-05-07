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
#include "mpi.h"

#include "reprompi_bench/caching/caching.h"

static const char MODULE_NAME[] = "None";

static void init_module(int argc, char** argv) {
};


static void empty(void) {
};


static void print_parameters(FILE* f) {
    fprintf(f, "#@clear-cache=%s\n", MODULE_NAME);
}

void register_warm_cache_module(reprompi_caching_module_t *cache_mod) {
  cache_mod->name = strdup(MODULE_NAME);
  cache_mod->type = REPROMPI_CLEAR_CACHE_NONE;

  cache_mod->init_module = init_module;
  cache_mod->cleanup_module = empty;

  cache_mod->clear_cache = empty;

  cache_mod->print_info = print_parameters;
}


