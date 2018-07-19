/*
 * SKaMPIClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <assert.h>
#include <mpi.h>

#include "SKaMPIClockSync.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"

SKaMPIClockSync::SKaMPIClockSync(ClockOffsetAlg *offsetAlg) {
  this->offset_alg = offsetAlg;
  this->tds = NULL;
}

SKaMPIClockSync::~SKaMPIClockSync() {
//  delete[] offset_algs;
  delete[] tds;
}


GlobalClock* SKaMPIClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {

  int my_rank, np;
  int i;
  double* tmp_tds;
  ClockOffset* offset = NULL;

  // TODO make this a parameter
  int nexchanges = 100;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  // TODO: mem leak, delete tds
  tds = new double[np];
  for (i = 0; i < np; i++)
      tds[i] = 0.0;

  //  measure ping-pong time between processes 0 and i
  for (i = 1; i < np; i++) {

    MPI_Barrier(comm);
    if (my_rank == 0){
      offset = offset_alg->measure_offset(comm, 0, i, c);
      tds[i] = offset->get_offset();
    } else if (my_rank == i) {
      offset = offset_alg->measure_offset(comm, 0, my_rank, c);
      tds[0] = offset->get_offset();
    }
  }

  // send root time differences to all the other processes
  tmp_tds =  new double[np];
  if (my_rank == 0) {
    for (i = 1; i < np; i++)
      tmp_tds[i] = tds[i];
  }

  assert(np - 1 >= 0);
  MPI_Bcast(&(tmp_tds[1]), np - 1, MPI_DOUBLE, 0, comm);

  // update local time differences to all other processes
  // TODO: remove local time diffs to other procs
 if (my_rank != 0) {
    for (i = 1; i < np; i++) {
      tds[i] = tmp_tds[i] + tds[0];
    }
  }
  MPI_Barrier(comm);

  delete[] tmp_tds;
  delete offset;
  return new GlobalClockOffset(c, tds[0]);
}



