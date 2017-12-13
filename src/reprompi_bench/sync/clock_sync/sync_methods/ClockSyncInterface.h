
#ifndef REPROMPIB_CLOCKSYNC_INTERFACE_H_
#define REPROMPIB_CLOCKSYNC_INTERFACE_H_

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"

class ClockSyncInterface{


public:

  ClockSyncInterface() {};
  virtual ~ClockSyncInterface() {};

  virtual Clock* synchronize_all_clocks(void) = 0;

};


#endif /*  REPROMPIB_CLOCKSYNC_INTERFACE_H_  */
