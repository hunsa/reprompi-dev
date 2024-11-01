
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#include "ClockOffsetAlg.h"
#include "PingpongClockOffsetAlg.h"

#include "reprompi_bench/sync/clock_sync/utils/sync_utils.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

PingpongClockOffsetAlg::PingpongClockOffsetAlg(int nexchanges_rtt, int nexchanges) {
  this->rtt            = -1.0;
  this->nexchanges_rtt = nexchanges_rtt;
  this->nexchanges     = nexchanges;
}

PingpongClockOffsetAlg::~PingpongClockOffsetAlg() {
}

ClockOffset* PingpongClockOffsetAlg::measure_offset(MPI_Comm comm, int ref_rank, int client_rank, Clock& clock) {
  // JK pingpongs
  int my_rank, nprocs; //, client_rank;
  int i;
  MPI_Status status;
  ClockOffset* offset = NULL;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);

  ZF_LOGV("%d: measure_offset start (%d - %d)", my_rank, ref_rank, client_rank);

  {
    int rank1;
    int rank2;
    if( ref_rank < client_rank ) {
      rank1 = ref_rank;
      rank2 = client_rank;
    } else {
      rank1 = client_rank;
      rank2 = ref_rank;
    }
    std::string key_str = std::to_string(rank1) + "," + std::to_string(rank2);
    if( (this->rankpair2rtt).find(key_str) != (this->rankpair2rtt).end()) {
      compute_rtt(ref_rank, client_rank, comm, 10, this->nexchanges_rtt, clock, &rtt);
      this->rankpair2rtt[key_str] = rtt;
    } else {
      rtt = this->rankpair2rtt[key_str];
    }
  }


 //   printf("%d: rtt %d %d %20.9f\n", my_rank, ref_rank, client_rank, rtt);
 // }

  if (my_rank == ref_rank) {
    double tlocal, tremote;
    for (i = 0; i < nexchanges; i++) {
      //printf("%d: ref recv from %d START\n", my_rank, client_rank);
      MPI_Recv(&tremote, 1, MPI_DOUBLE, client_rank, 0, comm, &status);
      tlocal = clock.get_time();
      MPI_Ssend(&tlocal, 1, MPI_DOUBLE, client_rank, 0, comm);
      //printf("%d: ref ssend to %d END %d\n", my_rank, client_rank, i);
    }

    //printf("myrank (ref) %d: pingpong offset (%d-%d) done\n", my_rank, ref_rank, client_rank);

  } else if (my_rank == client_rank) {
    double *time_var, *local_time, *time_var2;
    double median, dummy, master_time;

    time_var = new double[nexchanges];
    time_var2 = new double[nexchanges];
    local_time = new double[nexchanges];

    for (i = 0; i < nexchanges; i++) {
      dummy = clock.get_time();
      //printf("%d: ssend to %d START\n", my_rank, ref_rank);
      MPI_Ssend(&dummy, 1, MPI_DOUBLE, ref_rank, 0, comm);
      //printf("%d: ssend to %d END\n", my_rank, ref_rank);
      MPI_Recv(&master_time, 1, MPI_DOUBLE, ref_rank, 0, comm, &status);
      //printf("%d: recv from %d END %d\n", my_rank, ref_rank, i);
      local_time[i] = clock.get_time();
      time_var[i] = local_time[i] - master_time - rtt / 2.0;
      time_var2[i] = time_var[i];
    }

    gsl_sort(time_var2, 1, nexchanges);

    if (nexchanges % 2 == 0) {
      median = gsl_stats_median_from_sorted_data(time_var2, 1, nexchanges - 1);
    } else {
      median = gsl_stats_median_from_sorted_data(time_var2, 1, nexchanges);
    }

    for (i = 0; i < nexchanges; i++) {
      if (time_var[i] == median) {
        offset = new ClockOffset(local_time[i], time_var[i]);
        break;
      }
    }

    delete[] time_var;
    delete[] time_var2;
    delete[] local_time;

    //printf("myrank %d: pingpong offset (%d-%d) done\n", my_rank, ref_rank, client_rank);
  }


  if (offset == NULL) {
    offset = new ClockOffset();
  }
  return offset;
}


