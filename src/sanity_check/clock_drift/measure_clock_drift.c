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

#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/misc.h"
#include "parse_drift_test_options.h"


#include <gsl/gsl_statistics.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_sort.h>

#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
//#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static const int RTT_WARMUP_ROUNDS = 5;
static const int OUTPUT_ROOT_PROC = 0;

void estimate_all_rtts(int master_rank, int other_rank, const int n_pingpongs,
        double *rtt) {
    int my_rank, np;
    MPI_Status stat;
    int i;
    double tmp;
    double *rtts = NULL;
    double mean;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == master_rank) {
        double tstart, tremote;

        /* warm up */
        for (i = 0; i < RTT_WARMUP_ROUNDS; i++) {
            tmp = get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD);
            MPI_Recv(&tmp, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD, &stat);
        }

        rtts = (double*) malloc(n_pingpongs * sizeof(double));

        for (i = 0; i < n_pingpongs; i++) {
            tstart = get_time();
            MPI_Send(&tstart, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD);
            MPI_Recv(&tremote, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD,
                    &stat);
            rtts[i] = get_time() - tstart;
        }

    } else if (my_rank == other_rank) {
        double tlocal = 0, troot;

        /* warm up */
        for (i = 0; i < RTT_WARMUP_ROUNDS; i++) {
            MPI_Recv(&tmp, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD,
                    &stat);
            tmp = get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD);
        }

        for (i = 0; i < n_pingpongs; i++) {
            MPI_Recv(&troot, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD,
                    &stat);
            tlocal = get_time();
            MPI_Send(&tlocal, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD);
        }
    }

    if (my_rank == master_rank) {
        double upperq;
        double cutoff_val;
        double *rtts2;
        int n_datapoints;

        gsl_sort(rtts, 1, n_pingpongs);

        upperq = gsl_stats_quantile_from_sorted_data(rtts, 1, n_pingpongs,
                0.75);
        cutoff_val = 1.5 * upperq;

        rtts2 = (double*) calloc(n_pingpongs, sizeof(double));
        n_datapoints = 0;
        for (i = 0; i < n_pingpongs; i++) {
            if (rtts[i] <= cutoff_val) {
                rtts2[i] = rtts[i];
                n_datapoints = i + 1;
            } else {
                break;
            }
        }

        mean = gsl_stats_mean(rtts2, 1, n_datapoints);

        free(rtts);
        free(rtts2);

        MPI_Send(&mean, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&mean, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD, &stat);
    }

    *rtt = mean;
}

static int min_int(const void* a, const void* b) {
  if (*(int*)a < *(int*)b) {
    return -1;
  } else if (*(int*)a == *(int*)b) {
    return 0;
  }
  return 1;
}

void generate_test_process_list(int process_ratio, int **testprocs_list_p, int* ntestprocs) {
  int *testprocs_list;
  int n;
  int my_rank, np;
  int max_power_two;
  int i;
  int index = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  max_power_two = (int)pow(2, floor(log2(np)));


  if (np == 1) {
    *ntestprocs = 0;
    *testprocs_list_p = NULL;
  } else {

  if (process_ratio == 100) {
    n = np - 1;   // print all processes
  } else {
    n = (int)((double)np * process_ratio/100);
  }

  if (n < 2) { // no need to generate random processes to test
    if (max_power_two == np) {
      n = 1;    // print the output for one process - the last rank
    } else {
      n = 2;  // print the output for 2 processes - the largest power of two and the last rank
    }
  }
  testprocs_list = (int*)calloc(n, sizeof(int));

  testprocs_list[0] = np-1;
  if (n > 1 && max_power_two != np) {
    testprocs_list[0] = max_power_two-1;
    testprocs_list[1] = np-1;
  }

  if (n >= np-1) {  // use all processes except the root for the clock drift tests
    index = 0;
    for (i=0; i<np; i++) {
      if (i != OUTPUT_ROOT_PROC) {
        testprocs_list[index++] = i;
      }
    }
  } else {
    if ((n>1 && max_power_two == np) || (n>2)) {
      if (my_rank == OUTPUT_ROOT_PROC) {
        int* tmpprocs_list;
        tmpprocs_list = (int*)calloc(np-1, sizeof(int));

        index = 0;
        for (i=0; i<np; i++) {
          if (i!= OUTPUT_ROOT_PROC && i!= max_power_two-1 && i!= np-1) {
            tmpprocs_list[index++] = i;     // all processes except the root are candidates for the clock drift tests
          }
        }
        // shuffle list of ranks
        shuffle(tmpprocs_list, index);

        // take the first n-2 ranks
        index = 1;  // at least one test rank is already set (np-1)
        if (max_power_two != np) {
          index = 2;  // the second test rank is set as well
        }
        for (i=index; i<n; i++) {
          testprocs_list[i] = tmpprocs_list[i-index];
        }
        free(tmpprocs_list);
      }

      qsort (testprocs_list, n, sizeof(int), min_int);
      // send test list to all processes
      MPI_Bcast(testprocs_list, n, MPI_INT, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);
    }
  }

  *ntestprocs = n;
  *testprocs_list_p = testprocs_list;
  }

#if ZF_LOG_LEVEL == ZF_LOG_VERBOSE
  if (my_rank == OUTPUT_ROOT_PROC) {
    ZF_LOGV("Number of ranks to test: %d", *ntestprocs);
    ZF_LOGV("Ranks: ");
    for (i=0; i<n; i++) {
      ZF_LOGV("%d ", (*testprocs_list_p)[i]);
    }
  }
#endif
}

void print_initial_settings(int argc, char* argv[], reprompib_drift_test_opts_t opts,
        print_sync_info_t print_sync_info) {
    int my_rank, np;
    FILE * f;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;

        fprintf(f, "#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            fprintf(f, " %s", argv[i]);
        }
        fprintf(f, "\n");
        fprintf(f, "#@nrep=%ld\n", opts.n_rep);
        fprintf(f, "#@steps=%d\n", opts.steps);
        fprintf(f, "#@timerres=%14.9f\n", MPI_Wtick());

        print_time_parameters(f);
        print_sync_info(f);
    }

}

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    int i;
    reprompib_drift_test_opts_t opts;
    int master_rank;
    MPI_Status stat;
    double* rtts_s = NULL;
    FILE* f;
    reprompib_sync_module_t clock_sync;

    double global_time, local_time, min_drift;

    double *all_local_times = NULL;
    double *all_global_times = NULL;
    double time_msg[2];

    int step;
    int n_wait_steps = 0;
    double wait_time_s = 1;
    struct timespec sleep_time;
    double runtime_s;
    int ntestprocs;
    int* testprocs_list;
    int index;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    reprompib_register_sync_modules();

    parse_drift_test_options(&opts, argc, argv);

    reprompib_init_sync_module(argc, argv, &clock_sync);
    init_timer();

    n_wait_steps = opts.steps + 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    generate_test_process_list(opts.print_procs_ratio, &testprocs_list, &ntestprocs);

    // compute RTTs
    if (my_rank == master_rank) {
      rtts_s = (double*) calloc(ntestprocs, sizeof(double));  // only need the list of RTTs for the master rank
      for (i = 0; i < ntestprocs; i++) {
            p = testprocs_list[i];    // select the process to test
            if (p != master_rank) {
                estimate_all_rtts(master_rank, p, opts.rtt_pingpongs_nrep, &rtts_s[i]);
            }
            ZF_LOGV("RTT between (%d - %d): %10.3f us", my_rank, p, 1e6 * rtts_s[i]);
        }
    } else {
      for (i = 0; i < ntestprocs; i++) {
        p = testprocs_list[i];    // make sure the current process is in the test list
        if (my_rank == p) {
          double tmp_rtt;
          estimate_all_rtts(master_rank, my_rank, opts.rtt_pingpongs_nrep, &tmp_rtt);
        }
      }
    }

    if (my_rank == master_rank) {
        all_global_times = (double*) calloc(ntestprocs * opts.n_rep * n_wait_steps,
                sizeof(double));
        all_local_times = (double*) calloc(ntestprocs * opts.n_rep * n_wait_steps,
                sizeof(double));
    }


    print_initial_settings(argc, argv, opts, clock_sync.print_sync_info);

    runtime_s = get_time();
    clock_sync.init_sync();

    clock_sync.sync_clocks();
    runtime_s = get_time() - runtime_s;

    if (my_rank == master_rank) {
        printf ("#@sync_duration=%14.9f\n", runtime_s);
    }
    for (step = 0; step < n_wait_steps; step++) {
        if (my_rank == master_rank) {
            for (index = 0; index < ntestprocs; index++) {
                p = testprocs_list[index];    // select the process to exchange pingpongs with

                for (i = 0; i < opts.n_rep; i++) {
                    if (p != master_rank) {
                        MPI_Send(&time_msg[0], 2, MPI_DOUBLE, p, 0,
                                MPI_COMM_WORLD);
                        MPI_Recv(&time_msg[0], 2, MPI_DOUBLE, p, 0,
                                MPI_COMM_WORLD, &stat);

                        // cannot use the local time on the root as a reference time, it needs to be
                        // normalized according to the synchronization method
                        // (for JK, SKaMPI, on the root process global_time  = local_time)
                        all_local_times[step * ntestprocs * opts.n_rep + index * opts.n_rep + i] =
                            clock_sync.get_global_time(get_time()) - rtts_s[index] / 2;
                        all_global_times[step * ntestprocs * opts.n_rep
                                         + index * opts.n_rep + i] = time_msg[1];

                    }
                }
                ZF_LOGV("[step %d] Measured drift between (%d - %d) into global array indices [%ld, %ld]",
                    step, my_rank, p,
                    step * ntestprocs * opts.n_rep + index * opts.n_rep,
                    step * ntestprocs * opts.n_rep + index * opts.n_rep + opts.n_rep-1);
            }

            // wait 1 second
            sleep_time.tv_sec = wait_time_s;
            sleep_time.tv_nsec = 0;

            nanosleep(&sleep_time, &sleep_time);

        } else {
          for (index = 0; index < ntestprocs; index++) {
            p = testprocs_list[index];    // make sure the current rank is in the test list

            if (my_rank == p) {
              for (i = 0; i < opts.n_rep; i++) {
                MPI_Recv(&time_msg[0], 2, MPI_DOUBLE, master_rank, 0,
                    MPI_COMM_WORLD, &stat);

                local_time = get_time();
                global_time = clock_sync.get_global_time(local_time);
                time_msg[0] = local_time;
                time_msg[1] = global_time;

                MPI_Send(&time_msg[0], 2, MPI_DOUBLE, master_rank, 0,
                    MPI_COMM_WORLD);
              }
            }
          }
        }

    }
    clock_sync.finalize_sync();

    f = stdout;
    if (my_rank == master_rank) {
      if (opts.print_procs_allpingpongs > 0) {
        fprintf(f,"%14s %3s %4s %14s %14s %14s\n", "wait_time_s", "p", "rep", "gtime", "reftime", "diff");
      } else {
        fprintf(f,"%14s %3s %14s\n", "wait_time_s", "p", "min_diff");
      }

    }

    for (step = 0; step < n_wait_steps; step++) {
      if (my_rank == master_rank) {
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // make sure the current rank is in the test list

          min_drift = -1;
          for (i = 0; i < opts.n_rep; i++) {
            global_time = all_global_times[step * ntestprocs
                                           * opts.n_rep + index * opts.n_rep + i];
            local_time = all_local_times[step * ntestprocs * opts.n_rep
                                         + index * opts.n_rep + i];

            if (opts.print_procs_allpingpongs > 0) {
              fprintf(f, "%14.9f %3d %4d %14.9f %14.9f %14.9f\n",
                step * wait_time_s, p, i, global_time,
                local_time, global_time - local_time);
            } else {
              if (min_drift < 0) {
                min_drift = fabs(global_time - local_time);
              } else {
                min_drift = repro_min(min_drift, fabs(global_time - local_time));
              }
            }
          }

          if (opts.print_procs_allpingpongs == 0) {
            fprintf(f, "%14.9f %3d %14.9f\n", step * wait_time_s, p, min_drift);
          }

        }
      }
    }

    if (my_rank == master_rank) {
        free(rtts_s);
        free(all_local_times);
        free(all_global_times);
    }

    free(testprocs_list);
    clock_sync.cleanup_module();
    reprompib_deregister_sync_modules();
    MPI_Finalize();
    return 0;
}
