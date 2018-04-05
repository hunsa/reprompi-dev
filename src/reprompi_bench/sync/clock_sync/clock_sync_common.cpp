/*
 * clock_sync_common.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: sascha
 */

#include <iostream>
#include <reprompi_bench/sync/clock_sync/clocks/GlobalClock.h>

#ifdef ENABLE_RDTSCP
#include <reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h>
#elif ENABLE_RDTSC
#include <reprompi_bench/sync/clock_sync/clocks/RdtscClock.h>
#else
#include <reprompi_bench/sync/clock_sync/clocks/MPIClock.h>
#endif

//#include "synchronization.h"

extern "C" {
#include "clock_sync_lib.h"
}

#include "clock_sync_common.h"

void default_init_synchronization(void) {}
void default_finalize_synchronization(void) {}

double default_get_normalized_time(double local_time, GlobalClock* global_clock) {
  double normtime = 0;

  if (global_clock != NULL) {
    normtime= global_clock->convert_to_global_time(local_time);
  } else {
    std::cerr <<"ERROR: No global time defined for this clock sync. method\n" << std::endl;
  }
  return normtime;
}

Clock* initialize_local_clock(void) {
  Clock* local_clock = NULL;
#ifdef ENABLE_RDTSCP
  local_clock = new RdtscpClock();
#elif ENABLE_RDTSC
  local_clock = new RdtscClock();
#else
  local_clock = new MPIClock();
#endif

  return local_clock;
}
