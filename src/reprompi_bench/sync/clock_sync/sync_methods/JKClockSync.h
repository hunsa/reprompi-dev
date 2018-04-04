#ifndef REPROMPIB_JKCLOCKSYNC_CLASS_H_
#define REPROMPIB_JKCLOCKSYNC_CLASS_H_

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"


class JKClockSync: public ClockSync {

private:
  int n_fitpoints; /* --fitpoints */
  int n_exchanges; /* --exchanges */
  ClockOffsetAlg* offset_alg;

public:
  JKClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, int n_exchanges);
  ~JKClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);

};

#endif /*  REPROMPIB_JKCLOCKSYNC_CLASS_H_  */
