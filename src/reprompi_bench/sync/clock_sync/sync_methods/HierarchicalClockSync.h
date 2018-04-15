
#ifndef REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_
#define REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClock.h"
#include "reprompi_bench/sync/clock_sync/utils/communicator_utils.h"
#include "ClockSync.h"

#include "reprompi_bench/sync/clock_sync/utils/hwloc_helpers.h"

class HierarchicalClockSync: public ClockSync {

private:
  ClockSync *syncInterNode;
  ClockSync *syncSocket;
  ClockSync *syncOnSocket;

public:
  HierarchicalClockSync(ClockSync *syncInterNode, ClockSync *syncSocket, ClockSync *syncOnSocket); //, SyncConfiguration& conf);
  ~HierarchicalClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_  */
