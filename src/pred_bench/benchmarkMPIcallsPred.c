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
#include <math.h>
#include <time.h>
#include "mpi.h"

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "benchmark_job.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_timing_options.h"
#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "parse_options.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "nrep_estimation.h"

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

static const int OUTPUT_ROOT_PROC = 0;
static const int HASHTABLE_SIZE=100;
static const char OUTPUT_FORMAT_STR[] = "%30s %10ld %10ld %10ld %20.9f %20.9f %11s %15.9f\n";

static int cmpfunc(const void * a, const void * b) {
  if (*(double*) a > *(double*) b) {
    return 1;
  } else if (*(double*) a < *(double*) b) {
    return -1;
  } else {
    return 0;
  }
}

static void print_measurement_results_prediction(const job_t* job_p, const reprompib_common_options_t* opts_p,
    double* maxRuntimes_sec, const nrep_pred_params_t* pred_params_p, const pred_conditions_t* conds_p,
    const long total_nrep) {

  int j;
  int error = 0;
  int my_rank, np;
  double mean_runtime_sec, median_runtime_sec;
  FILE* f;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  f = stdout;
  if (my_rank == OUTPUT_ROOT_PROC) {
    if (opts_p->output_file != NULL) {
      f = fopen(opts_p->output_file, "a");
      if (f == NULL) {
        error = 1;
      }
    }
  }

  if (error) {
    reprompib_print_error_and_exit("Cannot open output file");
  }

  if (my_rank == OUTPUT_ROOT_PROC) {
    if (job_p->n_rep == 0) {   // no correct results
      // runtime_sec = 0;
      mean_runtime_sec = 0;
      median_runtime_sec = 0;
    } else {      // print the last measured runtime
      //runtime_sec = maxRuntimes_sec[job_p->n_rep - 1];
      mean_runtime_sec = gsl_stats_mean(maxRuntimes_sec, 1, job_p->n_rep);
      qsort(maxRuntimes_sec, job_p->n_rep, sizeof(double), cmpfunc);
      median_runtime_sec = gsl_stats_median_from_sorted_data(maxRuntimes_sec, 1, job_p->n_rep);
    }

    for (j = 0; j < conds_p->n_methods; j++) {
      fprintf(f, OUTPUT_FORMAT_STR, get_call_from_index(job_p->call_index), total_nrep, job_p->n_rep, job_p->count,
          mean_runtime_sec, median_runtime_sec, get_prediction_methods_list()[pred_params_p->info[j].method],
          conds_p->conditions[j]);
    }

    if (opts_p->output_file != NULL) {
      fclose(f);
    }
  }

}

static void print_initial_settings_prediction_to_file(FILE* f, const nrep_pred_params_t* pred_params_p,
    print_sync_info_t print_sync_info) {
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank == OUTPUT_ROOT_PROC) {
    int i;
    fprintf(f, "#@pred_nrep_min=%ld\n", pred_params_p->n_rep_min);
    fprintf(f, "#@pred_nrep_max=%ld\n", pred_params_p->n_rep_max);
    fprintf(f, "#@pred_nrep_stride=%ld\n", pred_params_p->n_rep_stride);
    fprintf(f, "#Prediction methods:\n");
    for (i = 0; i < pred_params_p->n_methods; i++) {
      fprintf(f, "#\t%s (thres=%f, win=%d)\n", get_prediction_methods_list()[pred_params_p->info[i].method],
          pred_params_p->info[i].method_thres, pred_params_p->info[i].method_win);
    }
    fprintf(f, "#\n");
  }

}

static void print_initial_settings_prediction(const reprompib_common_options_t* common_opts_p,
    const nrep_pred_params_t* pred_params_p, const reprompib_dictionary_t* dict,
    print_sync_info_t print_sync_info, const reprompi_timing_method_t runtime_type) {
  FILE* f;
  int my_rank;
  int error = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  print_common_settings(common_opts_p, print_sync_info, dict, runtime_type);

  print_initial_settings_prediction_to_file(stdout, pred_params_p, print_sync_info);

  if (my_rank == OUTPUT_ROOT_PROC) {
    if (common_opts_p->output_file != NULL) {
      f = fopen(common_opts_p->output_file, "a");
      if (f != NULL) {
        print_initial_settings_prediction_to_file(f, pred_params_p, print_sync_info);
      } else {
        error = 1;
      }
    } else {
      f = stdout;
    }
  }

  if (error) {
    reprompib_print_error_and_exit("Cannot open output file");
  }

  if (my_rank == OUTPUT_ROOT_PROC) {
    fprintf(f, "%30s %10s %10s %10s %20s %20s %11s %15s\n",
              "test", "total_nrep", "valid_nrep", "count", "mean_runtime_sec",
              "median_runtime_sec", "pred_method", "pred_value");
    if (common_opts_p->output_file != NULL) {
      fclose(f);
    }
  }
}


static void compute_runtimes_prediction(double* tstart_sec, double* tend_sec, long current_start_index, long current_nreps,
    const reprompib_sync_module_t* sync_module, reprompi_timing_method_t runtime_type, double* maxRuntimes_sec, long* updated_nreps) {

  int my_rank;
  int* sync_errorcodes = NULL;
  long i;
  double* tmp_runtimes = NULL;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  compute_runtimes(current_nreps, tstart_sec, tend_sec, OUTPUT_ROOT_PROC,
      sync_module, runtime_type, &tmp_runtimes, &sync_errorcodes);

  if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
    // remove measurements that resulted in an window error
    long nreps = 0;

    if (my_rank == OUTPUT_ROOT_PROC) {
      for (i = 0; i < current_nreps; i++) {
        if (sync_errorcodes[i] == 0) {
          maxRuntimes_sec[nreps] = tmp_runtimes[i];
          nreps++;
        }
      }

      // the runtimes arrays are only allocated on the root process
      free(sync_errorcodes);
      free(tmp_runtimes);
    }

    *updated_nreps = nreps;
  }
  else {
    if (my_rank == OUTPUT_ROOT_PROC) {
      for (i = 0; i < current_nreps; i++) {
            maxRuntimes_sec[i] = tmp_runtimes[i];
      }

      // the runtimes arrays are only allocated on the root process
      free(tmp_runtimes);
    }

    *updated_nreps = current_nreps;
  }

}

int main(int argc, char* argv[]) {
  int my_rank, procs;
  long i, jindex, current_index, runtimes_index;
  double* tstart_sec;
  double* tend_sec;
  double* maxRuntimes_sec = NULL;
  job_list_t jlist;
  collective_params_t coll_params;
  long nrep, stride;
  int stop_meas;
  pred_conditions_t pred_coefs;
  basic_collective_params_t coll_basic_info;
  time_t start_time, end_time;
  double* batch_runtimes;
  long updated_batch_nreps;
  reprompib_dictionary_t params_dict;
  reprompib_common_options_t common_opt;
  nrep_pred_params_t pred_opts;

  reprompib_sync_module_t sync_module;
  reprompib_sync_params_t sync_params;
  reprompi_timing_method_t runtime_type;

  /* start up MPI
   * */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  // initialize time measurement functions
  init_timer();
  start_time = time(NULL);

  reprompib_register_sync_modules();

  // initialize global dictionary
  reprompib_init_dictionary(&params_dict, HASHTABLE_SIZE);

  // parse arguments and set-up benchmarking jobs
  print_command_line_args(argc, argv);

  // parse the benchmark-specific arguments (prediction methods)
  reprompib_parse_options(argc, argv, &pred_opts);

  // parse common arguments (e.g., msizes list, MPI calls to benchmark, input file)
  reprompib_parse_common_options(&common_opt, argc, argv);

  // parse timing options
  reprompib_parse_timing_options(&runtime_type, argc, argv);

  // parse extra parameters into the global dictionary
  reprompib_parse_extra_key_value_options(&params_dict, argc, argv);

  // initialize synchronization module
  reprompib_init_sync_module(argc, argv, &sync_module);

  init_collective_basic_info(common_opt, procs, &coll_basic_info);
  //generate_pred_job_list(&pred_opts, &common_opt, &jlist);
  generate_job_list(&common_opt, 0, &jlist);

  // execute the benchmark jobs
  for (jindex = 0; jindex < jlist.n_jobs; jindex++) {
    job_t job;
    job = jlist.jobs[jlist.job_indices[jindex]];

    if (jindex == 0) {
      print_initial_settings_prediction(&common_opt, &pred_opts, &params_dict,
          sync_module.print_sync_info, runtime_type);
    }

    tstart_sec = (double*) malloc(pred_opts.n_rep_max * sizeof(double));
    tend_sec = (double*) malloc(pred_opts.n_rep_max * sizeof(double));

    maxRuntimes_sec = (double*) malloc(pred_opts.n_rep_max * sizeof(double));

    nrep = pred_opts.n_rep_min;
    stride = pred_opts.n_rep_stride;

    current_index = 0;
    runtimes_index = 0;

    collective_calls[job.call_index].initialize_data(coll_basic_info, job.count, &coll_params);

    // initialize synchronization
    sync_params.nrep = pred_opts.n_rep_max;
    sync_module.init_sync(&sync_params);

    sync_module.sync_clocks();      // compute clock drift models relative to the root node
    sync_module.init_sync_round();  // broadcast first window

    while (1) {

      // main measurement loop
      for (i = 0; i < nrep; i++) {
        sync_module.start_sync();

        tstart_sec[current_index] = get_time();
        collective_calls[job.call_index].collective_call(&coll_params);
        tend_sec[current_index] = get_time();
        current_index++;
        runtimes_index++;

        sync_module.stop_sync();
      }

      batch_runtimes = maxRuntimes_sec + (runtimes_index - nrep);
      compute_runtimes_prediction(tstart_sec, tend_sec, (current_index - nrep), nrep, &sync_module,
          runtime_type, batch_runtimes, &updated_batch_nreps);

      if (my_rank==OUTPUT_ROOT_PROC) {
        // set the number of correct measurements to take into account out-of-window measurement errors
        runtimes_index = (runtimes_index - nrep) + updated_batch_nreps;

        stop_meas = 0;
        set_prediction_conditions(runtimes_index, maxRuntimes_sec, pred_opts, &pred_coefs);
        stop_meas = check_prediction_conditions(pred_opts, pred_coefs);
      }
      // make sure all processes have the same decision to stop the measurements
      MPI_Bcast(&stop_meas, 1, MPI_INT, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

      if (stop_meas == 1) {
        break;
      } else {
        nrep = nrep + stride;
        stride = stride * 2;
      }

      if (current_index + nrep > pred_opts.n_rep_max) {
        nrep = pred_opts.n_rep_max - current_index;
      }
      if (current_index >= pred_opts.n_rep_max) {
        break;
      }
    }
    job.n_rep = runtimes_index;
    // print_results
    print_measurement_results_prediction(&job, &common_opt, maxRuntimes_sec,
        &pred_opts, &pred_coefs, current_index);

    free(tstart_sec);
    free(tend_sec);
    free(maxRuntimes_sec);

    collective_calls[job.call_index].cleanup_data(&coll_params);
    sync_module.finalize_sync();
  }

  end_time = time(NULL);
  print_final_info(&common_opt, start_time, end_time);

  cleanup_job_list(jlist);
  reprompib_free_common_parameters(&common_opt);
  reprompib_cleanup_dictionary(&params_dict);
  sync_module.cleanup_module();

  reprompib_deregister_sync_modules();
  /* shut down MPI */
  MPI_Finalize();

  return 0;
}
