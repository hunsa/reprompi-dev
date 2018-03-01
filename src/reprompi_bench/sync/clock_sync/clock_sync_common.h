/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/
#ifndef REPROMPIB_CLOCK_SYNCHRONIZATION_COMMON_H_
#define REPROMPIB_CLOCK_SYNCHRONIZATION_COMMON_H_

#include <iostream>
#include <reprompi_bench/sync/clock_sync/clocks/GlobalClock.h>

#ifdef ENABLE_RDTSCP
#include <reprompi_bench/sync/clock_sync/clocks/RdtscpClock.h>
#elif ENABLE_RDTSC
#include <reprompi_bench/sync/clock_sync/clocks/RdtscClock.h>
#else
#include <reprompi_bench/sync/clock_sync/clocks/MPIClock.h>
#endif

#include "synchronization.h"

extern "C" {
#include "clock_sync_lib.h"
}


inline void default_init_synchronization(void) {}
inline void default_finalize_synchronization(void) {}

inline double default_get_normalized_time(double local_time, Clock* global_clock) {
  GlobalClock* lmclock = NULL;
  if (global_clock != NULL) {
    lmclock = dynamic_cast<GlobalClock*>(global_clock);
    if (lmclock != NULL) {
      return lmclock->convert_to_global_time(local_time);
    }
  }

  if (global_clock == NULL || lmclock == NULL) {
    std::cerr <<"ERROR: No global time defined for this clock sync. method\n" << std::endl;
  }
  return 0;
}

inline Clock* initialize_local_clock(void) {
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

#endif /* REPROMPIB_CLOCK_SYNCHRONIZATION_COMMON_H_ */
