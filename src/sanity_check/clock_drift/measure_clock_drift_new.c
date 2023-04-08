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

// avoid getsubopt bug
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <time.h>
#include <math.h>
#include "mpi.h"

#include <getopt.h>

#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/misc.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static const int OUTPUT_ROOT_PROC = 0;
static int Minimum_ping_pongs =  20;
static int Number_ping_pongs  =  100;

double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank, reprompib_sync_module_t *clock_sync);

typedef struct opt {
  long n_rep; /* --nrep */
  int steps;  /* --steps */
  char testname[256];
  double print_procs_ratio;  /* --print-procs-ratio */
} reprompib_drift_test_opts_t;

static const struct option default_long_options[] = {
    { "nrep", required_argument, 0, 'n' },
    { "steps", required_argument, 0, 's' },
    { "print-procs-ratio", required_argument, 0, 'p' },
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
};

int parse_drift_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv);

void print_help(char* testname) {
    int my_rank;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == root_proc) {

        printf("\nUSAGE: %s [options]\n", testname);

        printf("options:\n");
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "--steps",
                    "set the number of 1s steps to wait after sync (default: 0)");
        printf("%-40s %-40s\n", "--nrep=<nrep>",
                    "set the number of ping-pong rounds between two processes to measure offset");
        printf("%-40s %-40s\n", "--print-procs-ratio",
        "set the fraction of the total processes to be tested for clock drift. If print-procs-ratio=0, only the last rank and the rank with the largest power of two are tested (default: 0)");

        printf("\nEXAMPLES: mpirun -np 4 %s --nrep=2 --clock-sync=HCA2 --print-procs-ratio=0.1\n", testname);
        printf("\nEXAMPLES: mpirun -np 4 %s --nrep=2 --clock-sync=HCA2 --steps=5 --print-procs-ratio=0.1\n", testname);
        printf("\n\n");
    }
}


void init_parameters(reprompib_drift_test_opts_t* opts_p, char* name) {
  opts_p->n_rep = 10;
  opts_p->steps = 0;
  opts_p->print_procs_ratio = 0;
  strcpy(opts_p->testname,name);
}


int parse_drift_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv) {
    int c;

    init_parameters(opts_p, argv[0]);

    opterr = 0;

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "h", default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 's': /* here only two steps, the first after sync and the second after 's' seconds */
            opts_p->steps = atoi(optarg);
            break;
        case 'p': /* fraction of processes for which to measure the drift (normal distribution)
                   if print_procs_ratio==0, print only the largest power of two and the last rank
                   */
            opts_p->print_procs_ratio = atof(optarg);
            break;
        case 'n': /* number of repetitions (pingpongs) */
            opts_p->n_rep = atol(optarg);
            break;
        case 'h':
            print_help(opts_p->testname);
            break;
        case '?':
            break;
        }
    }

    if (opts_p->steps < 0) {
      reprompib_print_error_and_exit("Invalid number of steps (should be >=0)");
    }
    if (opts_p->print_procs_ratio < 0 || opts_p->print_procs_ratio > 1) {
      reprompib_print_error_and_exit("Invalid process ratio (should be a number between 0 and 1)");
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options

    return 0;
}

static int min_int(const void* a, const void* b) {
  if (*(int*)a < *(int*)b) {
    return -1;
  } else if (*(int*)a == *(int*)b) {
    return 0;
  }
  return 1;
}

void generate_test_process_list(double process_ratio, int **testprocs_list_p, int* ntestprocs) {
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

  if (process_ratio == 1) {
    n = np - 1;   // print all processes
  } else {
    n = (int)((double)np * process_ratio);
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
    reprompib_drift_test_opts_t opts;
    int master_rank;
    FILE* f;
    reprompib_sync_module_t clock_sync;

    double  min_drift;
    double *all_global_times = NULL;

    //int step;
    int n_wait_steps = 0;
    //double wait_time_s = 1;
    //struct timespec sleep_time;
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
  REPROMPI_init_timer();

    n_wait_steps = 2;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    generate_test_process_list(opts.print_procs_ratio, &testprocs_list, &ntestprocs);

    if (my_rank == master_rank) {
      all_global_times = (double*) calloc(ntestprocs * n_wait_steps, sizeof(double));
    }

    print_initial_settings(argc, argv, opts, clock_sync.print_sync_info);

    runtime_s = REPROMPI_get_time();
    clock_sync.init_sync();

    clock_sync.sync_clocks();
    runtime_s = REPROMPI_get_time() - runtime_s;

    Number_ping_pongs = opts.n_rep;

    if (my_rank == master_rank) {
      double target_time = REPROMPI_get_time() + ((double)opts.steps);
      int first_iter = 1;

      printf ("#@sync_duration=%14.9f\n", runtime_s);

      int offset = 0;

      // measure once
      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];    // select the process to exchange pingpongs with
        if (p != master_rank) {
          all_global_times[offset * ntestprocs + index] = SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &clock_sync);
        }
      }

      //printf("target_time=%14.9f, cur_time=%14.9f\n", target_time, REPROMPI_get_time());
      // wait until 's' seconds are done
      do {
        double cur_time = REPROMPI_get_time();
        if( cur_time >= target_time ) {
          if( first_iter == 1 ) {
            // we were late.. report a warning
            printf ("#@LATE_WARNING=%d\n", opts.steps);
          }
          break;
        } else {
          first_iter = 0;
        }
      } while( 1 );

      // measure again
      offset = 1;
      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];    // select the process to exchange pingpongs with
        if (p != master_rank) {
          all_global_times[offset * ntestprocs + index] = SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &clock_sync);
        }
      }


    } else {
      int i;

      for(i=0; i<n_wait_steps; i++) {
        // measure twice (n_wait_steps should be 2)
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // make sure the current rank is in the test list
          if (my_rank == p) {
            SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &clock_sync);
          }
        }
      }
    }
    clock_sync.finalize_sync();

    f = stdout;
    if (my_rank == master_rank) {
      int offset = 0;

      fprintf(f,"%14s %3s %14s\n", "wait_time_s", "p", "min_diff");

      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];
        min_drift = all_global_times[offset * ntestprocs + index];
        fprintf(f, "%14.9f %3d %14.9f\n", 0.0f, p, fabs(min_drift));
      }

      offset = 1;
      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];
        min_drift = all_global_times[offset * ntestprocs + index];
        fprintf(f, "%14.9f %3d %14.9f\n", (double)opts.steps, p, fabs(min_drift));
      }

      free(all_global_times);
    }

    free(testprocs_list);
    clock_sync.cleanup_module();
    reprompib_deregister_sync_modules();
    MPI_Finalize();
    return 0;
}



double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank, reprompib_sync_module_t *clock_sync) {
  // SKaMPI pingpongs
  int i; //, other_global_id;
  double s_now, s_last, t_last, t_now;
  double td_min, td_max;
  double invalid_time = -1.0;
  MPI_Status status;
  int pp_tag = 43;
  int my_rank;

  double return_offset = 0.0;

  double ping_pong_min_time; /* ping_pong_min_time is the minimum time of one ping_pong
   between the root node and the , negative value means
   time not yet determined;
   needed to avoid measuring again all the 100 RTTs when re-synchronizing
   (in this case only a few ping-pongs are performed if the RTT stays
   within 110% for the ping_pong_min_time)
   */

  MPI_Comm_rank(comm, &my_rank);

  //printf("nb_ping_pongs: %d\n", Number_ping_pongs);

  // check whether I am participating here
  // if not, there is no offset
  if (my_rank != client_rank && my_rank != ref_rank) {
    return 0.0;
  }

  // check whether we have ping_pong_min_time in our hash
  // if so, take it and use it (can stop after min_n_ping_pong rounds)
  // if not, we set ping_pong_min_time to -1.0 (then we need to do n_ping_pongs rounds)
  //  int rank1;
  //  int rank2;
  //  if( ref_rank < client_rank ) {
  //    rank1 = ref_rank;
  //    rank2 = client_rank;
  //  } else {
  //    rank1 = client_rank;
  //    rank2 = ref_rank;
  //  }

  ping_pong_min_time = -1.0;

  /* I had to unroll the main loop because I didn't find a portable way
   to define the initial td_min and td_max with INFINITY and NINFINITY */
  if (my_rank == ref_rank) {

    s_last = clock_sync->get_global_time(REPROMPI_get_time());
    MPI_Send(&s_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
    MPI_Recv(&t_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm, &status);
    s_now = clock_sync->get_global_time(REPROMPI_get_time());
    MPI_Send(&s_now, 1, MPI_DOUBLE, client_rank, pp_tag, comm);

    td_min = t_last - s_now;
    td_max = t_last - s_last;

  } else {
    //other_global_id = ref_rank;

    MPI_Recv(&s_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
    t_last = clock_sync->get_global_time(REPROMPI_get_time());
    MPI_Send(&t_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
    MPI_Recv(&s_now, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
    t_now = clock_sync->get_global_time(REPROMPI_get_time());

    td_min = s_last - t_last;
    td_min = repro_max(td_min, s_now - t_now);

    td_max = s_now - t_last;
  }
  if (my_rank == ref_rank) {
    i = 1;
    while (1) {

      MPI_Recv(&t_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm, &status);
      if (t_last < 0.0) {
        break;
      }

      s_last = s_now;
      s_now = clock_sync->get_global_time(REPROMPI_get_time());

      td_min = repro_max(td_min, t_last - s_now);
      td_max = repro_min(td_max, t_last - s_last);

      if (ping_pong_min_time >= 0.0 && i >= Minimum_ping_pongs && s_now - s_last < ping_pong_min_time * 1.10) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
        break;
      }

      i++;
      if (i >= Number_ping_pongs) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
        break;
      }
      MPI_Send(&s_now, 1, MPI_DOUBLE, client_rank, pp_tag, comm);

    }
  } else {
    i = 1;
    while (1) {
      MPI_Send(&t_now, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
      MPI_Recv(&s_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
      t_last = t_now;
      t_now = clock_sync->get_global_time(REPROMPI_get_time());

      if (s_last < 0.0) {
        break;
      }
      td_min = repro_max(td_min, s_last - t_now);
      td_max = repro_min(td_max, s_last - t_last);

      if (ping_pong_min_time >= 0.0 && i >= Minimum_ping_pongs && t_now - t_last < ping_pong_min_time * 1.10) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
        break;
      }
      i++;
    }
  }

  if (my_rank == ref_rank) {
    return_offset = (td_min + td_max) / 2.0;
  } else {
    return_offset = -(td_min + td_max) / 2.0;
  }

  return return_offset;
}



