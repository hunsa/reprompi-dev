
#ifndef REPROMPIB_SKAMPICLOCKSYNC_CLASS_H_
#define REPROMPIB_SKAMPICLOCKSYNC_CLASS_H_

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "ClockSync.h"


class SKaMPIClockSync : public ClockSync {

private:
   double *tds; /* tds[i] is the time difference between the
   current node and global node i */
   ClockOffsetAlg* offset_alg;

public:
	SKaMPIClockSync(ClockOffsetAlg *offsetAlg);
  ~SKaMPIClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);

};




#endif /*  REPROMPIB_SKAMPICLOCKSYNC_CLASS_H_  */
