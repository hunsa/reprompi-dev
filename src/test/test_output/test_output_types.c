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
#include <time.h>
#include <math.h>
#include "mpi.h"

#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reprompi_bench/output_management/results_output.h"
#include "reprompi_bench/option_parser/parse_timing_options.h"

static const int OUTPUT_ROOT_PROC = 0;

typedef struct bench_results {
  double* local_start_time;
  double* local_end_time;
  double* global_start_time;
  double* global_end_time;
  int* errorcodes;
  int nrep;
} bench_results_t;

static bench_results_t bench_results;
static const double epsilon = 1e-10;

static double mock_get_global_time(double local_time) {
  int i, my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  for (i=0; i<bench_results.nrep; i++) {
    if (fabs(local_time - bench_results.local_start_time[i]) < epsilon ) {   // the provided local_time is a start timestamp
        return bench_results.global_start_time[i];
    }
    if (fabs(local_time - bench_results.local_end_time[i]) < epsilon ) {   // the provided local_time is an end timestamp
        return bench_results.global_end_time[i];
    }
  }

  fprintf(stderr, "ERROR: Cannot identify a global time for the provided timestamp: rank=%d local_time=%10.9f\n", my_rank, local_time);
  MPI_Finalize();
  exit(0);
}

static int* mock_get_errorcodes(void) {
  return bench_results.errorcodes;
}

static void read_input_file(char* file_name, bench_results_t* bench_res) {
  FILE* file;
  int result = 0;
  int expected_result = 0;
  int i;
  int nprocs, p, my_rank, input_procs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  file = fopen(file_name, "r");
  if (file) {
    // the file contains data for input_procs processes, with nrep_p repetitions for each of them
    result = fscanf(file, "%d %d", &input_procs, &(bench_res->nrep));
    expected_result = 2;

    if (result != expected_result) /* incorrectly formatted file */
    {
      fprintf(stderr, "ERROR: Incorrectly formatted input file: %s\n", file_name);
      MPI_Finalize();
      exit(0);
    }

    bench_res->local_start_time = (double*) calloc(bench_res->nrep, sizeof(double));
    bench_res->local_end_time = (double*) calloc(bench_res->nrep, sizeof(double));
    bench_res->global_start_time = (double*) calloc(bench_res->nrep, sizeof(double));
    bench_res->global_end_time = (double*) calloc(bench_res->nrep, sizeof(double));

    bench_res->errorcodes = (int*) calloc(bench_res->nrep, sizeof(int));

    for (p = 0; p < nprocs; p++) {  // read data for all process, but only store
                                    // the timestamps corresponding to the current process
      for (i = 0; i < bench_res->nrep; i++) {
        double t1, t2, tg1, tg2;
        int e1;

        result = fscanf(file, "%lf %lf %lf %lf %d", &t1, &t2, &tg1, &tg2, &e1);

        /* number of job parameters: tstart[i] tend[i] errorcode*/
        expected_result = 5;
        if (result != expected_result) /* incorrectly formatted file */
        {
          fprintf(stderr, "ERROR: Incorrectly formatted input file: %s\n", file_name);
          MPI_Finalize();
          exit(0);
        }

        if (p == 0 || p == my_rank) { // store data corresponding to the current rank
                                      // (or to process 0 if not all processes have data in the input file)
          bench_res->local_start_time[i] = t1;
          bench_res->local_end_time[i] = t2;
          bench_res->global_start_time[i] = tg1;
          bench_res->global_end_time[i] = tg2;
          bench_res->errorcodes[i] = e1;
        }
      }
      if (p == input_procs - 1) { // no data left in the file
        if (p < my_rank) {
          fprintf(stderr, "WARNING: No data for process %d in the input file. Using the data specified for process 0\n", my_rank);
          fflush(stderr);
        }
        break;
      }
    }
    fclose(file);
  } else {
    fprintf(stderr, "ERROR: Cannot open input file: %s\n", file_name);
    MPI_Finalize();
    exit(0);
  }
}

int main(int argc, char* argv[]) {
  int my_rank, procs;
  reprompib_options_t opts;
  reprompib_sync_module_t sync_module;
  reprompi_timing_method_t runtime_type;
  FILE* f = stdout;
  job_t job;
  int summarize;
  char* input_file;

  /* start up MPI
   *
   * */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  if (argc < 6) {
    if (my_rank == OUTPUT_ROOT_PROC) {
      printf("USAGE: mpirun -np 4 %s proc_sync_type runtime_type verbose summarize input_file\n", argv[0]);
      printf("\tArguments:\n");
      printf("\tproc_sync_type = {0,1}, 0=window-based, 1=barrier\n");
      printf("\truntime_type = [0-2], 0=local_times, 1=global_times, 2=local_avg(osu)\n");
      printf("\tverbose = {0,1}\n");
      printf("\tsummarize = {0,1}\n");
    }
    MPI_Finalize();
    exit(0);
  }

  summarize = atoi(argv[4]);
  input_file = strdup(argv[5]);
  read_input_file(input_file, &bench_results);

  // init job
  job.n_rep = bench_results.nrep;
  job.msize = 1;
  job.count = 1;
  job.call_index = 0;

  // init other options
  opts.n_rep = job.n_rep;
  opts.print_summary_methods = 0;
  opts.verbose = atoi(argv[3]);


  runtime_type = atoi(argv[2]);

  // init sync module
  sync_module.name = "test";
  sync_module.procsync = atoi(argv[1]);;

  sync_module.get_global_time = mock_get_global_time;
  sync_module.get_errorcodes = mock_get_errorcodes;

  if (summarize == 0) {
    print_measurement_results(f, job, bench_results.local_start_time, bench_results.local_end_time,
        &sync_module, &opts, runtime_type);
  }
  else {
    opts.print_summary_methods = 0xFF;
    print_summary(stdout, job, bench_results.local_start_time, bench_results.local_end_time,
        &sync_module, &opts, runtime_type);
  }

  free(bench_results.local_start_time);
  free(bench_results.local_end_time);
  free(bench_results.global_start_time);
  free(bench_results.global_end_time);
  free(bench_results.errorcodes);
  /* shut down MPI */
  MPI_Finalize();

  return 0;
}
