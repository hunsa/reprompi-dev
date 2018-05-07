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

#ifndef REPROMPIB_CACHING_H_
#define REPROMPIB_CACHING_H_


typedef enum {
  REPROMPI_CLEAR_CACHE_NONE = 0,
  REPROMPI_CLEAR_CACHE_MEMSET,
} reprompi_caching_type_t;


typedef void (*reprompi_caching_print_info_t)(FILE* f);

typedef struct reprompib_caching_module{
    void (*init_module)(int argc, char** argv);
    void (*cleanup_module)(void);

    void (*clear_cache)(void);

    reprompi_caching_print_info_t print_info;

    char* name;
    // a module is uniquely identified by the caching method
    reprompi_caching_type_t type;
} reprompi_caching_module_t;


void reprompib_register_caching_modules(void);
void reprompib_deregister_caching_modules(void);

void reprompib_init_caching_module(int argc, char** argv, reprompi_caching_module_t* caching_strat);
void reprompib_cleanup_caching_module(reprompi_caching_module_t* caching_strat);

void register_warm_cache_module(reprompi_caching_module_t *caching_strat);
void register_clear_cache_memset_module(reprompi_caching_module_t *caching_strat);

#endif /* REPROMPIB_CACHING_H_ */
