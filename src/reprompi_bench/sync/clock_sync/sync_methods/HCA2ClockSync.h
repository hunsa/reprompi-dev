#ifndef REPROMPIB_HCA2CLOCKSYNC_CLASS_H_
#define REPROMPIB_HCA2CLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCAClockSync.h"


class HCA2ClockSync: public HCAClockSync {     // Hierarchical synchronization in log2(p) steps

private:
  bool recompute_intercept;

protected:
  void remeasure_intercept_call_back(MPI_Comm comm, Clock &c, LinModel* lm, int client, int p_ref);
  void remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank);

public:
  HCA2ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, bool recompute_intercept);
  ~HCA2ClockSync();

  //GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  REPROMPIB_HCACLOCKSYNC2_CLASS_H_  */
