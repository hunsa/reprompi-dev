/*
 * clock_sync_parser.hpp
 *
 *  Created on: May 2, 2018
 *      Author: sascha
 */

#ifndef REPROMPI_BENCH_SYNC_CLOCK_SYNC_CLOCK_SYNC_LOADER_HPP_
#define REPROMPI_BENCH_SYNC_CLOCK_SYNC_CLOCK_SYNC_LOADER_HPP_

#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"

class ClockSyncLoader {

public:
  ClockSync* instantiate_clock_sync(const char *param_name);

};


#endif /* REPROMPI_BENCH_SYNC_CLOCK_SYNC_CLOCK_SYNC_LOADER_HPP_ */
