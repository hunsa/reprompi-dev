
#ifndef REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_
#define REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClock.h"
#include "reprompi_bench/sync/clock_sync/utils/communicator_utils.h"
#include "ClockSync.h"

#include "reprompi_bench/sync/clock_sync/utils/hwloc_helpers.h"

class HierarchicalClockSync: public ClockSync {

private:
  BaseClockSync *syncInterNode;
  BaseClockSync *syncSocket;
  BaseClockSync *syncOnSocket;

  MPI_Comm comm_internode;
  MPI_Comm comm_intranode;
  MPI_Comm comm_intersocket;
  MPI_Comm comm_intrasocket;

  bool comm_initialized;

  void initialized_communicators(MPI_Comm comm);

public:
  HierarchicalClockSync(BaseClockSync *syncInterNode, BaseClockSync *syncSocket, BaseClockSync *syncOnSocket); //, SyncConfiguration& conf);
  ~HierarchicalClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_  */
