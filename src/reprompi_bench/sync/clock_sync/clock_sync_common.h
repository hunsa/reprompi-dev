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

#include <reprompi_bench/sync/clock_sync/clocks/GlobalClock.h>
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
    fprintf(stderr, "ERROR: No global time defined for this clock sync. method\n");
  }
  return 0;
}

#endif /* REPROMPIB_CLOCK_SYNCHRONIZATION_COMMON_H_ */
