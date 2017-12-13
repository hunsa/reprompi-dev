
#ifndef REPROMPIB_SKAMPICLOCKSYNC_CLASS_H_
#define REPROMPIB_SKAMPICLOCKSYNC_CLASS_H_

#include <assert.h>
#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "ClockSync.h"


template<class OffsetAlgType>
class SKaMPIClockSync : public ClockSync {

private:
   double *tds; /* tds[i] is the time difference between the
   current node and global node i */
   OffsetAlgType* offset_algs;

public:
	SKaMPIClockSync(MPI_Comm comm, Clock* c);
  ~SKaMPIClockSync();

  Clock* synchronize_all_clocks(void);

};


template<class OffsetAlgType>
inline SKaMPIClockSync<OffsetAlgType>::SKaMPIClockSync(MPI_Comm comm, Clock* c) :
ClockSync(comm, c)
{
  int nprocs, i, my_rank;
  int root = 0;

  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &my_rank);

  tds = new double[nprocs];
  for (i = 0; i < nprocs; i++)
      tds[i] = 0.0;

  offset_algs = new OffsetAlgType[nprocs];
  for (i=0; i<nprocs; i++) {
    offset_algs[i] = OffsetAlgType(i, my_rank, comm, 100);
  }

}

template<class OffsetAlgType>
inline SKaMPIClockSync<OffsetAlgType>::~SKaMPIClockSync() {
  delete[] tds;
  delete[] offset_algs;
}


template<class OffsetAlgType>
inline Clock* SKaMPIClockSync<OffsetAlgType>::synchronize_all_clocks(void) {

  int my_rank, np;
  int i;
  double* tmp_tds;
  ClockOffset* offset = NULL;

  MPI_Comm_rank(this->comm, &my_rank);
  MPI_Comm_size(this->comm, &np);

  //  measure ping-pong time between processes 0 and i
  for (i = 1; i < np; i++) {

    MPI_Barrier(this->comm);
    if (my_rank == 0){
      offset = offset_algs[i].measure_offset(0,  *(this->local_clock));
      tds[i] = offset->get_offset();
    } else if (my_rank == i) {
      offset = offset_algs[0].measure_offset(0,  *(this->local_clock));
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
  MPI_Bcast(&(tmp_tds[1]), np - 1, MPI_DOUBLE, 0, this->comm);

  // update local time differences to all other processes
  // TODO: remove local time diffs to other procs
 if (my_rank != 0) {
    for (i = 1; i < np; i++) {
      tds[i] = tmp_tds[i] + tds[0];
    }
  }
  MPI_Barrier(this->comm);

  delete[] tmp_tds;
  delete offset;
  return new GlobalClockOffset(this->local_clock, tds[0]);
}


#endif /*  REPROMPIB_SKAMPICLOCKSYNC_CLASS_H_  */
