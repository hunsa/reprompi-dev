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

#ifndef REPROMPIB_BENCH_INFO_OUTPUT_H_
#define REPROMPIB_BENCH_INFO_OUTPUT_H_

#include <time.h>
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_timing_options.h"
#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/process_sync/process_synchronization.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bench_print_info {
  reprompib_sync_module_t* clock_sync;
  reprompib_proc_sync_module_t* proc_sync;
  reprompib_timing_method_t timing_method;
} reprompib_bench_print_info_t;

void print_command_line_args(int argc, char* argv[]);
void print_common_settings(const reprompib_bench_print_info_t* print_info,
    const reprompib_common_options_t* opts); //, const reprompib_dictionary_t* dict);
void print_common_settings_to_file(FILE* f, const reprompib_bench_print_info_t* print_info);
//,   const reprompib_dictionary_t* dict);
void print_final_info(const reprompib_common_options_t* opts, const time_t start_time, const time_t end_time);

#ifdef __cplusplus
}
#endif

#endif /* REPROMPIB_BENCH_INFO_OUTPUT_H_ */
