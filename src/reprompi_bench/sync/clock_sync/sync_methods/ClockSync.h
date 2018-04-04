
#ifndef REPROMPIB_CLOCKSYNC_CLASS_H_
#define REPROMPIB_CLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClock.h"

class ClockSync {

//protected:
//	MPI_Comm comm;
//	Clock* local_clock;

public:

  virtual GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c) = 0;
  virtual ~ClockSync() {};

};

class LinModel {
public:
  double slope;
  double intercept;
};


#endif /*  REPROMPIB_CLOCKSYNC_CLASS_H_  */
