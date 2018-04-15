#ifndef REPROMPIB_HCA3CLOCKSYNC_CLASS_H_
#define REPROMPIB_HCA3CLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"

class HCA3ClockSync: public ClockSync {

private:
  int n_fitpoints; /* --fitpoints */
  int n_exchanges; /* --exchanges */
  ClockOffsetAlg* offset_alg;
//  lm_t lm;

public:
  HCA3ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, int n_exchanges);
  ~HCA3ClockSync();

  LinModel learn_model(MPI_Comm comm, const int root_rank, const int other_rank, Clock& current_clock);
  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
};

#endif /*  REPROMPIB_HCA3CLOCKSYNC_CLASS_H_  */
