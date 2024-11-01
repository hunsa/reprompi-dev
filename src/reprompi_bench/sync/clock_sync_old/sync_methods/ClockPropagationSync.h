#ifndef REPROMPIB_CLOCKPROPAGATIONSYNC_CLASS_H_
#define REPROMPIB_CLOCKPROPAGATIONSYNC_CLASS_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "ClockSync.h"


/*
 * rank 0 has the correct clock (offset etc.)
 * propagate that model to all other processes in this same communicator
 */

class ClockPropagationSync: public BaseClockSync {

public:
  enum class ClockType { CLOCK_OFFSET, CLOCK_LM };

  ClockPropagationSync(ClockType clock_type);
  ~ClockPropagationSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);
  GlobalClock* create_global_dummy_clock(MPI_Comm comm, Clock& c);

private:
  ClockType clock_type;

};

#endif
