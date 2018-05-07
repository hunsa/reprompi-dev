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
#include <time.h>
#include "mpi.h"

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/process_sync/process_synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "benchmark_job.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_timing_options.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reprompi_bench/output_management/results_output.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/caching/caching.h"

#define MY_MAX(x, y) (((x) > (y)) ? (x) : (y))

static const int OUTPUT_ROOT_PROC = 0;

static void print_initial_settings(const reprompib_options_t* opts, const reprompib_common_options_t* common_opts,
    const reprompib_dictionary_t* dict, const reprompib_bench_print_info_t* print_info) {
    int my_rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    print_common_settings(print_info, common_opts, dict);

    if (my_rank == OUTPUT_ROOT_PROC) {
        FILE* f;

        f = stdout;
        if (opts->n_rep > 0) {
          fprintf(f, "#@nrep=%ld\n", opts->n_rep);
          if (common_opts->output_file != NULL) {
            f = fopen(common_opts->output_file, "a");
            fprintf(f, "#@nrep=%ld\n", opts->n_rep);
            fflush(f);
            fclose(f);
          }
        }
    }
}


static void reprompib_print_bench_output(job_t job, double* tstart_sec, double* tend_sec,
        const reprompib_options_t* opts, const reprompib_common_options_t* common_opts,
        const reprompib_bench_print_info_t* print_info) {
    FILE* f = stdout;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (common_opts->output_file != NULL) {
            f = fopen(common_opts->output_file, "a");
        }
    }

    if (opts->print_summary_methods >0)  {
        print_summary(stdout, job, tstart_sec, tend_sec, print_info, opts);
        if (common_opts->output_file != NULL) {
            print_measurement_results(f, job, tstart_sec, tend_sec, print_info, opts);
        }

    }
    else {
        print_measurement_results(f, job, tstart_sec, tend_sec, print_info, opts);
    }

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (common_opts->output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }

}


static void reprompib_parse_bench_options(int argc, char** argv) {
    int c;
    opterr = 0;

    const struct option bench_long_options[] = {
        { "help", required_argument, 0, 'h' },
        { 0, 0, 0, 0 }
    };

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "h", bench_long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 'h': /* list of summary options */
            reprompib_print_benchmark_help();
            break;
        case '?':
            break;
        }
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options
}



int main(int argc, char* argv[]) {
    int my_rank, procs;
    long i, jindex;
    double* tstart_sec;
    double* tend_sec;
    reprompib_options_t opts;
    reprompib_common_options_t common_opts;
    job_list_t jlist;
    collective_params_t coll_params;
    basic_collective_params_t coll_basic_info;
    time_t start_time, end_time;
    reprompib_bench_print_info_t print_info;

    reprompib_sync_module_t clock_sync;
    reprompib_proc_sync_module_t proc_sync;
    reprompib_sync_params_t sync_params;
    reprompib_timing_method_t runtime_type;
    reprompi_caching_module_t caching_module;
    long total_n_rep;
    int is_invalid;

    /* start up MPI
     *
     * */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    reprompib_register_sync_modules();
    reprompib_register_proc_sync_modules();

    reprompib_register_caching_modules();

    // initialize time measurement functions
    init_timer();
    start_time = time(NULL);


    // parse arguments and set-up benchmarking jobs
    print_command_line_args(argc, argv);

    reprompib_parse_bench_options(argc, argv);  // only "-h" for help

    // parse common arguments (e.g., msizes list, MPI calls to benchmark, input file)
    reprompib_parse_common_options(&common_opts, argc, argv);

    // parse timing options
    reprompib_parse_timing_options(&runtime_type, argc, argv);

    // parse the benchmark-specific arguments (nreps, summary)
    reprompib_parse_options(&opts, argc, argv);

    // initialize caching strategies
    reprompib_init_caching_module(argc, argv, &caching_module);

    // initialize synchronization module
    reprompib_init_sync_module(argc, argv, &clock_sync);
    reprompib_init_proc_sync_module(argc, argv, &clock_sync, &proc_sync);

    if (common_opts.input_file == NULL && opts.n_rep <=0) { // make sure nrep is specified when there is no input file
      reprompib_print_error_and_exit("The number of repetitions is not defined (specify the \"--nrep\" command-line argument or provide an input file)\n");
    }
    generate_job_list(&common_opts, opts.n_rep, &jlist);


    init_collective_basic_info(common_opts, procs, &coll_basic_info);
    // execute the benchmark jobs
    for (jindex = 0; jindex < jlist.n_jobs; jindex++) {
        job_t job;
        job = jlist.jobs[jlist.job_indices[jindex]];

        tstart_sec = (double*) malloc(job.n_rep * sizeof(double));
        tend_sec = (double*) malloc(job.n_rep * sizeof(double));

        collective_calls[job.call_index].initialize_data(coll_basic_info, job.count, &coll_params);

        // initialize synchronization
        sync_params.nrep = job.n_rep;
        proc_sync.init_sync(&sync_params);
        clock_sync.init_sync();


        print_info.clock_sync = &clock_sync;
        print_info.proc_sync = &proc_sync;
        print_info.timing_method = runtime_type;
        if (jindex == 0) {
            //print_initial_settings(&opts, &common_opts, &params_dict, &print_info);
            print_results_header(&print_info, &opts, common_opts.output_file, opts.verbose);
        }

        clock_sync.sync_clocks();
        proc_sync.init_sync_round();         // broadcast first window

        i = 0;
        total_n_rep = 0;
        while(1) {
          proc_sync.start_sync();

          tstart_sec[i] = get_time();
          collective_calls[job.call_index].collective_call(&coll_params);
          tend_sec[i] = get_time();

          is_invalid = proc_sync.stop_sync();
          if (is_invalid == REPROMPI_INVALID_MEASUREMENT) { // redo the measurement if we haven't reached the max possible number of reps
            //ZF_LOGV("[%d] invalid_measurement at i=%ld", my_rank, total_n_rep);
            if (total_n_rep >= opts.max_n_rep) {
              job.n_rep = i; // record the current number of correct reps
              break;
            }
          } else if( is_invalid == REPROMPI_OUT_OF_TIME_VALID ) {
            job.n_rep = i;
            break;
          } else if( is_invalid == REPROMPI_OUT_OF_TIME_INVALID ) {
            job.n_rep = MY_MAX(0, i-1);
            break;
          } else {
            i++;
          }
          total_n_rep++;
          if (i == job.n_rep) {
            break;
          }

          // apply cache cleaning strategy (if enabled)
          caching_module.clear_cache();
        }

        //print summarized data
        reprompib_print_bench_output(job, tstart_sec, tend_sec, &opts, &common_opts, &print_info);

        clock_sync.finalize_sync();
        proc_sync.finalize_sync();

        free(tstart_sec);
        free(tend_sec);

        collective_calls[job.call_index].cleanup_data(&coll_params);
    }


    end_time = time(NULL);
    print_final_info(&common_opts, start_time, end_time);

    cleanup_job_list(jlist);
    reprompib_free_common_parameters(&common_opts);
    reprompib_free_parameters(&opts);
//    reprompib_cleanup_dictionary(&params_dict);
    clock_sync.cleanup_module();
    proc_sync.cleanup_module();

    caching_module.cleanup_module();

    reprompib_deregister_sync_modules();
    reprompib_deregister_proc_sync_modules();
    reprompib_deregister_caching_modules();
    /* shut down MPI */
    MPI_Finalize();

    return 0;
}
