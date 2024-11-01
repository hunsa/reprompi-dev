#ifndef REPROMPI_HCA3OFFSETCLOCKSYNC_H
#define REPROMPI_HCA3OFFSETCLOCKSYNC_H

#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"

/**
 *
 * essentially the same algorithm as HCA3, except that we only learn the clock offset
 */

class HCA3OffsetClockSync : public BaseClockSync {

public:
  HCA3OffsetClockSync(ClockOffsetAlg *offsetAlg);
  ~HCA3OffsetClockSync();
  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
  GlobalClock* create_global_dummy_clock(MPI_Comm comm, Clock& c);

private:
  ClockOffsetAlg *offset_alg;
};


#endif //REPROMPI_HCA3OFFSETCLOCKSYNC_H
