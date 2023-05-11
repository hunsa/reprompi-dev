//
// Created by Sascha on 5/11/23.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "clock_drift_utils.h"
#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/time_measurement.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

int Minimum_ping_pongs =  20;
int Number_ping_pongs  =  100;
const int OUTPUT_ROOT_PROC = 0;

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
