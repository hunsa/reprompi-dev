#ifndef REPROMPIB_HCA3CLOCKSYNC_CLASS_H_
#define REPROMPIB_HCA3CLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/HCAAbstractClockSync.h"

class HCA3ClockSync: public HCAAbstractClockSync {

private:
  bool recompute_intercept;

protected:
  void remeasure_intercept_call_back(MPI_Comm comm, Clock &c,LinModel* lm, int client, int p_ref);
  void remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank);

public:
  HCA3ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, bool recompute_intercept);
  ~HCA3ClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  REPROMPIB_HCA3CLOCKSYNC_CLASS_H_  */
