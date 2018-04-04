
#ifndef REPROMPIB_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_CLOCKOFFSETALG_CLASS_H_

#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "ClockOffset.h"

class ClockOffsetAlg {

public:
  ClockOffsetAlg();

  virtual ClockOffset* measure_offset(MPI_Comm comm, int ref_rank, int other_rank, int nexchanges, Clock& clock) = 0;
  virtual ~ClockOffsetAlg() = 0;

};

#endif /*  REPROMPIB_CLOCKOFFSETALG_CLASS_H_  */
