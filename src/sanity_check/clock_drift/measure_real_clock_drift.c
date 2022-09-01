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



/**
 *
 *
 * This program should help us to determine the clock drift between
 * different MPI processes over time.
 *
 * It will repeat the following
 *  - perform X ping-pongs with each participating process
 *  - compute the minimum offset and store it for this timestamp
 *  - wait Y milliseconds
 *
 *
 */


// avoid getsubopt bug
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

#include <getopt.h>

//#include "reprompi_bench/sync/clock_sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/misc.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

static const int OUTPUT_ROOT_PROC = 0;
static int WARMUP_ROUNDS = 10;
static int Minimum_ping_pongs =  1;
static int Number_ping_pongs  =  5;

double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank);

typedef struct opt {
  long n_rep; /* --nrep */
  int nwaits;  /* --nwaits */
  int wait_ms;
  char testname[256];
} reprompib_drift_test_opts_t;

typedef struct res_str {
  int rank;
  int rep;
  double time_at_root;
  double offset_to_rank;
} res_str_t;

static const struct option default_long_options[] = {
    { "nrep", required_argument, 0, 'n' },      // number of ping pongs in SKaMPI-offset
    { "nwaits", required_argument, 0, 'r' },      // how many seconds to record
    { "wait_ms", required_argument, 0, 'w' },   // how many milli seconds to wait
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
};

int parse_drift_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv);

void print_help(char* testname) {
    int my_rank;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == root_proc) {

        if (strstr(testname, "measure_clock_drift") != 0) {
            printf("\nUSAGE: %s [options] [steps]\n", testname);
        }
        else {
            printf("\nUSAGE: %s [options]\n", testname);
        }

        printf("options:\n");
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "--nwaits",
                    "how many times to record (default: 60)");
        printf("%-40s %-40s\n", "--nrep=<nrep>",
                    "set the number of ping-pong rounds between two processes to measure offset (default: 5)");
        printf("%-40s %-40s\n", "--wait_ms",
                    "how many milli seconds to wait between offset computations (default: 500)");
        printf("\n\n");
    }
}


void init_parameters(reprompib_drift_test_opts_t* opts_p, char* name) {
  opts_p->n_rep    = 5;
  opts_p->nwaits  = 60;
  opts_p->wait_ms = 500;
  strcpy(opts_p->testname, name);
}


int parse_drift_test_options(reprompib_drift_test_opts_t* opts_p, int argc, char **argv) {
    int c;

    init_parameters(opts_p, argv[0]);

    opterr = 0;


    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "r:w:n:h", default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 'r':
            opts_p->nwaits = atoi(optarg);
            break;
        case 'w':
            opts_p->wait_ms = atoi(optarg);
            break;
        case 'n':
            opts_p->n_rep = atol(optarg);
            break;
        case 'h':
            print_help(opts_p->testname);
            break;
        case '?':
            break;
        }
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options

    return 0;
}


void print_initial_settings(int argc, char* argv[], reprompib_drift_test_opts_t opts) {
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
    fprintf(f, "#@nwaits=%d\n", opts.nwaits);
    fprintf(f, "#@wait_ms=%d\n", opts.wait_ms);
    fprintf(f, "#@timerres=%14.9f\n", MPI_Wtick());

    print_time_parameters(f);
  }
}

int main(int argc, char* argv[]) {
    int i;
    int my_rank, nprocs, p;
    reprompib_drift_test_opts_t opts;
    int master_rank;
    FILE* f;

    double cur_offset;
    res_str_t *clock_offsets = NULL;

    int nrows;
    struct timespec ts;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    parse_drift_test_options(&opts, argc, argv);

    init_timer();

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);


    nrows = opts.nwaits * (nprocs-1);

    if (my_rank == master_rank) {
      clock_offsets = (res_str_t*) calloc(nrows, sizeof(res_str_t));
    }

    print_initial_settings(argc, argv, opts);

    Number_ping_pongs = opts.n_rep;

    ts.tv_sec  = (long) opts.wait_ms / 1000;
    ts.tv_nsec = (long) (opts.wait_ms % 1000) * 1E6;

    /* warm up */
    {
      double tmp;
      MPI_Status stat;

      if( my_rank == master_rank ) {
        for(p=0; p<nprocs; p++) {
          if( p != master_rank ) {
            for (i = 0; i < WARMUP_ROUNDS; i++) {
              tmp = get_time();
              MPI_Send(&tmp, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
              MPI_Recv(&tmp, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD, &stat);
            }
          }
        }
      } else {

        for (i = 0; i < WARMUP_ROUNDS; i++) {
          MPI_Recv(&tmp, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &stat);
          tmp = get_time();
          MPI_Send(&tmp, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
      }
    } // end warm up


    for(i=0; i<opts.nwaits; i++) {
      for (p = 0; p < nprocs; p++) {
        if (p != master_rank) {
          if( my_rank == master_rank ) {
            res_str_t *res_pointer;

            cur_offset = SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p);
            res_pointer = &(clock_offsets[(p-1)*opts.nwaits + i]);

            res_pointer->rank = p;
            res_pointer->rep  = i;
            res_pointer->time_at_root = get_time();
            res_pointer->offset_to_rank = cur_offset;

          } else {
            SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p);
          }
        }
      }
      // now wait for some time
      if( my_rank == master_rank ) {
        nanosleep(&ts, NULL);
      }
    }

    f = stdout;
    if (my_rank == master_rank) {


      fprintf(f,"%3s %3s %s %21s\n", "r", "p", "time_ref", "offset");
      for (i = 0; i < nrows; i++) {
        fprintf(f, "%3d %3d %14.9f %14.9f\n", clock_offsets[i].rep, clock_offsets[i].rank,
            clock_offsets[i].time_at_root, clock_offsets[i].offset_to_rank);
      }

      free(clock_offsets);

    }

    MPI_Finalize();
    return 0;
}



double SKaMPIClockOffset_measure_offset(MPI_Comm comm, int ref_rank, int client_rank) {
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

    s_last = get_time();
    MPI_Send(&s_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
    MPI_Recv(&t_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm, &status);
    s_now = get_time();
    MPI_Send(&s_now, 1, MPI_DOUBLE, client_rank, pp_tag, comm);

    td_min = t_last - s_now;
    td_max = t_last - s_last;

  } else {
    //other_global_id = ref_rank;

    MPI_Recv(&s_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
    t_last = get_time();
    MPI_Send(&t_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
    MPI_Recv(&s_now, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
    t_now = get_time();

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
      s_now = get_time();

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
      t_now = get_time();

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



