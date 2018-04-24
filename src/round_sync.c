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
#include <assert.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <mpi.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/misc.h"
#include "benchmark_job.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "collective_ops/collectives.h"

#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
//#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

typedef struct {
  long bcast_n_rep; /* --bcast-nrep how many bcasts to do for the mean */
  double bcast_multiplier; /* --bcast-mult multiplier for mean Bcast time */
  int bcast_meas;     /* --bcast-meas how to compute the bcast run-time (mean, median, max) */
  long n_rep;
} reprompi_roundsync_params_t;


typedef struct {
  long max_n_rep;
  long n_rep;
  int verbose;
} reprompi_bench_params_t;

#define MAX_NREP 10
#define BCAST_MUTIPLIER 1.2
#define BCAST_NREP 10

static const int OUTPUT_ROOT_PROC = 0;

enum {
  REPROMPI_ARGS_PROCSYNC_BCAST_MULTIPLIER = 1200,
  REPROMPI_ARGS_PROCSYNC_BCAST_NREP,
  REPROMPI_ARGS_PROCSYNC_BCAST_MEASURE
};
enum {
  REPROMPI_ARGS_NREP = 1300,
  REPROMPI_ARGS_MAX_NREP,
  REPROMPI_ARGS_VERBOSE
};


enum {
  BCAST_MEASURE_MEAN = 0,
  BCAST_MEASURE_MEDIAN,
  BCAST_MEASURE_MAX
};

//static reprompib_sync_module_t* clock_sync_mod; /* pointer to current clock synchronization module */

// options specified from the command line
static reprompi_roundsync_params_t parameters;
static int invalid;
static double start_sync;
static double bcast_runtime = 0;

void roundsync_parse_options(int argc, char **argv, reprompi_roundsync_params_t* opts_p) {
  int c;

  static const struct option reprompi_sync_long_options[] = {
      { "bcast-mult", required_argument, 0, REPROMPI_ARGS_PROCSYNC_BCAST_MULTIPLIER },
      { "bcast-nrep", required_argument, 0, REPROMPI_ARGS_PROCSYNC_BCAST_NREP },
      { "bcast-meas", required_argument, 0, REPROMPI_ARGS_PROCSYNC_BCAST_MEASURE },
      { 0, 0, 0, 0 } };
  static const char reprompi_sync_opts_str[] = "";

  opts_p->bcast_multiplier = BCAST_MUTIPLIER;
  opts_p->bcast_n_rep = BCAST_NREP;
  opts_p->bcast_meas = BCAST_MEASURE_MEAN;

  optind = 1;
  optopt = 0;
  opterr = 0; // ignore invalid options
  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, reprompi_sync_opts_str, reprompi_sync_long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case REPROMPI_ARGS_PROCSYNC_BCAST_MULTIPLIER:
      opts_p->bcast_multiplier = atof(optarg);
      break;
    case REPROMPI_ARGS_PROCSYNC_BCAST_NREP:
      opts_p->bcast_n_rep = atol(optarg);
      break;
    case REPROMPI_ARGS_PROCSYNC_BCAST_MEASURE:
      if (strcmp(optarg, "mean") == 0) {
        opts_p->bcast_meas = BCAST_MEASURE_MEAN;
      }
      if (strcmp(optarg, "median") == 0) {
        opts_p->bcast_meas = BCAST_MEASURE_MEDIAN;
      }
      if (strcmp(optarg, "max") == 0) {
        opts_p->bcast_meas = BCAST_MEASURE_MAX;
      }
      break;
    case '?':
      break;
    }
  }

  // check for errors
  if (opts_p->bcast_multiplier < 1.0) {
    reprompib_print_error_and_exit("bcast-mult error: bcast multiplier must be >= 1.0");
  }
  if (opts_p->bcast_n_rep <= 0) {
    reprompib_print_error_and_exit("bcast-nrep error: we need to do at least 1 bcast");
  }

  optind = 1; // reset optind to enable option re-parsing
  opterr = 1; // reset opterr
}



void parse_bench_options(int argc, char **argv, reprompi_bench_params_t* opts_p) {
  int c;

  static const struct option reprompi_bench_long_options[] = {
      { "nrep", required_argument, 0, REPROMPI_ARGS_NREP },
      { "max-nrep", required_argument, 0, REPROMPI_ARGS_MAX_NREP },
      { "v", required_argument, 0, REPROMPI_ARGS_VERBOSE},
      { 0, 0, 0, 0 } };
  static const char reprompi_bench_opts_str[] = "";

  opts_p->n_rep = 0;
  opts_p->max_n_rep = 10;
  opts_p->verbose = 0;

  optind = 1;
  optopt = 0;
  opterr = 0; // ignore invalid options
  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, reprompi_bench_opts_str, reprompi_bench_long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case REPROMPI_ARGS_NREP:
      opts_p->n_rep = atol(optarg);
      break;
    case REPROMPI_ARGS_MAX_NREP:
      opts_p->max_n_rep = atol(optarg);
      break;
    case REPROMPI_ARGS_VERBOSE:
      opts_p->verbose = atoi(optarg);
      break;
    case '?':
      break;
    }
  }

  // check for errors
  if (opts_p->n_rep <= 0) {
    reprompib_print_error_and_exit("nrep error: nrep must be >= 1");
  }
  if (opts_p->max_n_rep < opts_p->n_rep) {
    reprompib_print_error_and_exit("max-nrep error: max-nrep must be >= nrep (default: 10)");
  }
  if (opts_p->verbose <= 0) {
    opts_p->verbose = 0;
  } else {
    opts_p->verbose = 1;
  }

  optind = 1; // reset optind to enable option re-parsing
  opterr = 1; // reset opterr
}



void compute_runtimes(int nrep, double* tstart_sec, double* tend_sec, int root_proc,
    double** maxRuntimes_sec_p) {

  double* maxRuntimes_sec;
  int i;
  double* local_runtimes;
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  maxRuntimes_sec = NULL;
  if (my_rank == OUTPUT_ROOT_PROC) {
    maxRuntimes_sec = (double*)calloc(nrep, sizeof(double));
  }

  local_runtimes = (double*)calloc(nrep, sizeof(double));
  for (i = 0; i < nrep; i++) {
    local_runtimes[i] = tend_sec[i] - tstart_sec[i];
  }
  // reduce local measurement results on the root
  MPI_Reduce(local_runtimes, maxRuntimes_sec, nrep, MPI_DOUBLE, MPI_MAX, root_proc, MPI_COMM_WORLD);

  free(local_runtimes);
  *maxRuntimes_sec_p = maxRuntimes_sec;
}


static void print_runtimes(FILE* f, job_t job, double* tstart_sec, double* tend_sec, int verbose) {
  double* maxRuntimes_sec = NULL;
  int i;
  int my_rank;
  int p, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (verbose) {
    double* all_tstart = NULL;
    double* all_tend = NULL;

    if (my_rank == OUTPUT_ROOT_PROC) {

      fprintf(f, "%50s %10s %10s %12s %14s %14s ", "test", "nrep", "proc", "msize", "start_time", "end_time");
      fprintf(f,  "%14s \n", "runtime_sec");

      all_tstart = (double*) calloc(job.n_rep*nprocs, sizeof(double));
      all_tend = (double*) calloc(job.n_rep*nprocs, sizeof(double));
    }

    MPI_Gather(tstart_sec, job.n_rep, MPI_DOUBLE, all_tstart, job.n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);
    MPI_Gather(tend_sec, job.n_rep, MPI_DOUBLE, all_tend, job.n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

    if (my_rank == OUTPUT_ROOT_PROC) {
      for (i = 0; i < job.n_rep; i++) {
        for (p=0; p< nprocs; p++){
          fprintf(f, "%50s %10d %10d %12ld %14.10f %14.10f %14.10f\n", get_call_from_index(job.call_index), i, p,
              job.msize, all_tstart[p*job.n_rep + i], all_tend[p*job.n_rep + i],
              all_tend[p*job.n_rep + i]-all_tstart[p*job.n_rep + i]);
        }
      }
    }

  } else {

    // maxRuntimes_sec and sync_errorcodes are only defined for the root process
    compute_runtimes(job.n_rep, tstart_sec, tend_sec, OUTPUT_ROOT_PROC,
        &maxRuntimes_sec);

    if (my_rank == OUTPUT_ROOT_PROC) {
      fprintf(f, "%50s %10s %12s ", "test", "nrep", "msize");
      fprintf(f,  "%14s \n", "runtime_sec");

      for (i = 0; i < job.n_rep; i++) {
        fprintf(f, "%50s %10d %12ld %14.10f\n", get_call_from_index(job.call_index), i,
            job.msize,
            maxRuntimes_sec[i]);
      }

      free(maxRuntimes_sec);
    }
  }
}



static void round_init_synchronization(const long n_rep) {
  int i;
  parameters.n_rep = n_rep;

  invalid = 0;
}

static void round_finalize_synchronization(void) {
  //free(invalid);
}

// initialize first window
static void round_init_sync_round(void) {
  int my_rank;
  double dummy_time;
  int master_rank = 0;
  int i=0;
  double* bcast_times = NULL;
  double bcast_mean_rt, bcast_median_rt;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  dummy_time = get_time();
  bcast_times = (double*)calloc(parameters.bcast_n_rep, sizeof(double));

  for (i=0; i< parameters.bcast_n_rep; i++) {
    bcast_times[i] = get_time();
    MPI_Bcast(&dummy_time, 1, MPI_DOUBLE, master_rank, MPI_COMM_WORLD);
    bcast_times[i] = get_time() - bcast_times[i];
  }

  gsl_sort(bcast_times, 1, parameters.bcast_n_rep);
  bcast_mean_rt = gsl_stats_mean(bcast_times, 1, parameters.bcast_n_rep);
  bcast_median_rt = gsl_stats_quantile_from_sorted_data (bcast_times, 1, parameters.bcast_n_rep, 0.5);
  ZF_LOGV("[rank %d] Bcast times [us] mean=%f median=%f min=%f max=%f", my_rank,
      1e6 *bcast_mean_rt, 1e6 *bcast_median_rt, 1e6 *bcast_times[0], 1e6 *bcast_times[parameters.bcast_n_rep-1]);
  free(bcast_times);

  switch(parameters.bcast_meas) {
  case BCAST_MEASURE_MAX:
    MPI_Reduce(&(bcast_times[parameters.bcast_n_rep-1]), &bcast_runtime, 1, MPI_DOUBLE, MPI_MAX, master_rank, MPI_COMM_WORLD);
    break;
  case BCAST_MEASURE_MEDIAN:
      MPI_Reduce(&bcast_median_rt, &bcast_runtime, 1, MPI_DOUBLE, MPI_MAX, master_rank, MPI_COMM_WORLD);
      break;
  case BCAST_MEASURE_MEAN:
  default:
      MPI_Reduce(&bcast_mean_rt, &bcast_runtime, 1, MPI_DOUBLE, MPI_MAX, master_rank, MPI_COMM_WORLD);
      break;
  }
  // make sure the root records the longest Bcast time

}

static void round_start_synchronization(void) {
  int is_first = 1;
  double global_time;
  int my_rank;
  int master_rank = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == master_rank) {
    start_sync = get_time() + bcast_runtime * parameters.bcast_multiplier;
  }
  MPI_Bcast(&start_sync, 1, MPI_DOUBLE, master_rank, MPI_COMM_WORLD);
  ZF_LOGV("[rank %d] current_time=%20.10f need_to_wait_us=%f", my_rank, get_time(), 1e6*(start_sync-get_time()));

  while (1) {
    global_time = get_time();

    if (global_time >= start_sync) {
      if (is_first == 1) {
        invalid = 1;
      }
      break;
    }
    is_first = 0;
  }
}

static int round_stop_synchronization(void) {
  double global_time;
  int current_meas_invalid = 0;

  MPI_Allreduce(&invalid, &current_meas_invalid, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  global_time = get_time();
  return current_meas_invalid;
}



int main(int argc, char* argv[]) {
  int my_rank, procs;
  long i; //jindex;
  double* tstart_sec;
  double* tend_sec;
  reprompi_bench_params_t opts;
  //reprompib_common_options_t common_opts;
  //job_list_t jlist;
  collective_params_t coll_params;
  basic_collective_params_t coll_basic_info;
  job_t job;
  int is_invalid;
  long total_n_rep = 0;

  /* start up MPI
   *
   * */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  parse_bench_options(argc, argv, &opts);

  job.call_index = MPI_ALLGATHER;
  job.count = 1024;
  job.msize = 1024;
  job.n_rep = opts.n_rep;

  coll_basic_info.datatype = MPI_BYTE;
  coll_basic_info.nprocs = procs;
  coll_basic_info.op = MPI_BOR;
  coll_basic_info.root = 0;

  tstart_sec = (double*) malloc(job.n_rep * sizeof(double));
  tend_sec = (double*) malloc(job.n_rep * sizeof(double));

  collective_calls[job.call_index].initialize_data(coll_basic_info, job.count, &coll_params);

  // initialize synchronization
  //sync_params.nrep = job.n_rep;
  //proc_sync.init_sync(&sync_params);

  roundsync_parse_options(argc, argv, &parameters);
  round_init_synchronization(job.n_rep);
  round_init_sync_round();         // broadcast first window

  // execute MPI call nrep times
  //for (i = 0; i < job.n_rep; i++) {
  i = 0;
  total_n_rep = 0;
  while(1) {
    round_start_synchronization();

    tstart_sec[i] = get_time();
    collective_calls[job.call_index].collective_call(&coll_params);
    tend_sec[i] = get_time();

    is_invalid = round_stop_synchronization();
    if (is_invalid) { // redo the measurement if we haven't reached the max possible number of reps
      //ZF_LOGV("[%d] invalid_measurement at i=%ld", my_rank, total_n_rep);

      if (total_n_rep >= opts.max_n_rep) {
        job.n_rep = i; // record the current number of correct reps
        break;
      }
    } else {
      i++;
    }
    total_n_rep++;
    if (i == job.n_rep) {
      break;
    }
  }

  print_runtimes(stdout, job, tstart_sec, tend_sec, opts.verbose);


  round_finalize_synchronization();

  free(tstart_sec);
  free(tend_sec);

  collective_calls[job.call_index].cleanup_data(&coll_params);



  /* shut down MPI */
  MPI_Finalize();

  return 0;
}

