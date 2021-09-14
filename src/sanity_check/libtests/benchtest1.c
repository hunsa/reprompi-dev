//
// Created by Sascha on 9/13/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
//@ add_includes
#include "reprompi_bench/sync/process_sync/process_synchronization.h"
#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/benchmark_lib/reproMPIbenchmark.h"
#include "reprompi_bench/sync/time_measurement.h"

int main(int argc, char *argv[]) {
  int i;
  void *send_buffer;
  int rank;

  int cnts[] = {10, 100, 1000};
  int ncnts = 3;
  //@ declare_variables
  reprompib_sync_module_t clock_sync;
  reprompib_proc_sync_module_t proc_sync;
  reprompib_sync_params_t sync_params;
  reprompib_options_t reprompib_opts;
  long reprompib_nrep_index;
  int cur_nrep;
  int is_invalid;
  reprompib_job_t reprompib_job;
  double *t2 = NULL;
  double *t1 = NULL;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  //MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

  //@ initialize_bench
  reprompib_initialize_benchmark(argc, argv, &reprompib_opts, &clock_sync, &proc_sync);
  //reprompib_opts.n_rep = 10;
  //@ initialize_timestamps t1
  t1 = (double *) calloc(reprompib_opts.n_rep, sizeof(double));
  //@ initialize_timestamps t2
  t2 = (double *) calloc(reprompib_opts.n_rep, sizeof(double));

  for (i = 0; i < ncnts; i++) {
    //@ set callname=MPI_Bcast

    send_buffer = malloc(cnts[i]);

    //@ start_measurement_loop
    sync_params.nrep = reprompib_opts.n_rep;
    sync_params.count = 0;
    proc_sync.init_sync(&sync_params);
    clock_sync.init_sync();

    clock_sync.sync_clocks();
    proc_sync.init_sync_round();

    cur_nrep = reprompib_opts.n_rep;
    reprompib_nrep_index = 0;
    while (1) {
      proc_sync.start_sync();

      //@ start_sync
      proc_sync.start_sync();

      //@ measure_timestamp t1
      t1[reprompib_nrep_index] = get_time();

      MPI_Bcast(send_buffer, cnts[i], MPI_BYTE, 0, MPI_COMM_WORLD);

      //@ measure_timestamp t2
      t2[reprompib_nrep_index] = get_time();

      //@ stop_sync
      proc_sync.stop_sync();

      //@stop_measurement_loop
      is_invalid = proc_sync.stop_sync();
      if (is_invalid == REPROMPI_INVALID_MEASUREMENT) {
        // redo the measurement
        // we are still in the time frame
      } else if (is_invalid == REPROMPI_OUT_OF_TIME_VALID) {
        cur_nrep = reprompib_nrep_index + 1;
        break;
      } else if (is_invalid == REPROMPI_OUT_OF_TIME_INVALID) {
        cur_nrep = MY_MAX(0, reprompib_nrep_index - 1);
        break;
      } else {
        reprompib_nrep_index++;
      }
      if (cur_nrep == reprompib_nrep_index) {
        break;
      }
    }


    //@ print_runtime_array name=runtime_per_process end_time=t2 start_time=t1 type=all size=size
    {
      reprompib_initialize_job(cur_nrep, t1, t2,
                               "", "runtime_per_process", "all", &reprompib_job);
      reprompib_add_ivar_to_job("count", cnts[i], &reprompib_job);
      reprompib_print_bench_output(&reprompib_job, &clock_sync, &proc_sync, &reprompib_opts);
      reprompib_cleanup_job(&reprompib_job);
    }

    free(send_buffer);
    send_buffer = NULL;
  }

  //@ cleanup_variables
  free(t2);
  free(t1);
  t2 = NULL;
  t1 = NULL;

  //@cleanup_bench
  reprompib_cleanup_benchmark(&reprompib_opts, &clock_sync, &proc_sync);

  MPI_Finalize();

  return 0;
}
