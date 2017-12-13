
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#include "ClockOffsetAlg.h"
#include "PingpongClockOffsetAlg.h"

#include "reprompi_bench/sync/clock_sync/utils/sync_utils.h"

PingpongClockOffsetAlg::PingpongClockOffsetAlg(int rank1, int rank2, MPI_Comm comm, int nexchanges) :
    ClockOffsetAlg(rank1, rank2, comm), nexchanges(nexchanges), rtt(-1) {
}

PingpongClockOffsetAlg::~PingpongClockOffsetAlg() {
}

ClockOffset* PingpongClockOffsetAlg::measure_offset(int ref_rank, Clock& clock) {  // JK pingpongs
  int my_rank, nprocs, client_rank;
  int i;
  MPI_Status status;
  ClockOffset* offset = NULL;

  MPI_Comm_rank(this->comm, &my_rank);
  MPI_Comm_size(this->comm, &nprocs);

  if (my_rank == this->rank1 || my_rank == this->rank2) {
    if (ref_rank == this->rank1) {
      client_rank = this->rank2;
    } else if (ref_rank == this->rank2) {
      client_rank = this->rank1;
    } else {
      return new ClockOffset();
    }

    if (rtt < 0) { // first time the method is called - we need to measure the RTT
      compute_rtt(ref_rank, client_rank, this->comm, 10, nexchanges, clock, &rtt);
      //printf("rtt %d %d %20.9f\n", ref_rank, client_rank, rtt);
    }

    if (my_rank == ref_rank) {

      double tlocal, tremote;
      for (i = 0; i < nexchanges; i++) {
        MPI_Recv(&tremote, 1, MPI_DOUBLE, client_rank, 0, this->comm, &status);
        tlocal = clock.get_time();
        MPI_Ssend(&tlocal, 1, MPI_DOUBLE, client_rank, 0, this->comm);
      }
    } else if (my_rank == client_rank) {
      double *time_var, *local_time, *time_var2;
      double median, dummy, master_time;

      time_var = new double[nexchanges];
      time_var2 = new double[nexchanges];
      local_time = new double[nexchanges];

      for (i = 0; i < nexchanges; i++) {
        dummy = clock.get_time();
        MPI_Ssend(&dummy, 1, MPI_DOUBLE, ref_rank, 0, this->comm);
        MPI_Recv(&master_time, 1, MPI_DOUBLE, ref_rank, 0, this->comm, &status);
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
    }
  }

  if (offset == NULL) {
    offset = new ClockOffset();
  }
  return offset;
}


