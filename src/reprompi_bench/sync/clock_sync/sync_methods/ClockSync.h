
#ifndef REPROMPIB_CLOCKSYNC_CLASS_H_
#define REPROMPIB_CLOCKSYNC_CLASS_H_

#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "ClockSyncInterface.h"


class ClockSync : public ClockSyncInterface{

protected:
	MPI_Comm comm;
	Clock* local_clock;

public:
	ClockSync(MPI_Comm comm, Clock* c);

  virtual Clock* synchronize_all_clocks(void) = 0;
  virtual ~ClockSync() {};

};

inline ClockSync::ClockSync(MPI_Comm comm, Clock* c):
  comm(comm),
  local_clock(c)
  {}

#endif /*  REPROMPIB_CLOCKSYNC_CLASS_H_  */
