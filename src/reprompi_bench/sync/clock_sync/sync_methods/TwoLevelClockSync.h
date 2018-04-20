
#ifndef REPROMPIB_TWO_LEVEL_CLOCKSYNC_CLASS_H_
#define REPROMPIB_TWO_LEVEL_CLOCKSYNC_CLASS_H_

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClock.h"
#include "reprompi_bench/sync/clock_sync/utils/communicator_utils.h"
#include "ClockSync.h"

class TwoLevelClockSync: public ClockSync {

private:
  ClockSync *syncInterNode;
  ClockSync *syncIntraNode;

  MPI_Comm comm_internode;
  MPI_Comm comm_intranode;

  bool comm_initialized;

  void initialized_communicators(MPI_Comm comm);

public:
  TwoLevelClockSync(ClockSync *syncInterNode, ClockSync *syncIntraNode);
  ~TwoLevelClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  REPROMPIB_TWO_LEVEL_CLOCKSYNC_CLASS_H_  */
