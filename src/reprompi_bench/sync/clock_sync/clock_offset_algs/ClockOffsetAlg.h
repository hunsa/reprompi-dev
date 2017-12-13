
#ifndef REPROMPIB_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_CLOCKOFFSETALG_CLASS_H_

#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "ClockOffset.h"

class ClockOffsetAlg {

protected:
	int rank1;   // the clock offset is computed relative to the reference rank in comm
	int rank2;
  MPI_Comm comm;

public:
  ClockOffsetAlg(int rank1 = 0, int rank2 = 0, MPI_Comm comm = MPI_COMM_NULL);

  virtual ClockOffset* measure_offset(int ref_rank, Clock& c) = 0;
  virtual ~ClockOffsetAlg();

};

#endif /*  REPROMPIB_CLOCKOFFSETALG_CLASS_H_  */
