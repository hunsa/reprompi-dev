/*
 * HCAAbstractClockSync.h
 *
 *  Created on: May 3, 2018
 *      Author: sascha
 */

#ifndef REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_HCAABSTRACTCLOCKSYNC_H_
#define REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_HCAABSTRACTCLOCKSYNC_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"

#include "ClockSync.h"

class HCAAbstractClockSync: public ClockSync {

private:


protected:
  int n_fitpoints; /* --fitpoints */
  ClockOffsetAlg *offset_alg;

  int my_pow_2(int exp);

  LinModel learn_model(MPI_Comm comm, Clock &c, const int root_rank, const int other_rank);
  LinModel merge_linear_models(LinModel lm1, LinModel lm2);

  void remeasure_intercept(MPI_Comm comm, Clock &c, LinModel* lm, int client, int p_ref);
  void remeasure_all_intercepts(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank);

  virtual void remeasure_intercept_call_back(MPI_Comm comm, Clock &c,LinModel* lm, int client, int p_ref) = 0;
  virtual void remeasure_all_intercepts_call_back(MPI_Comm comm, Clock &c, LinModel* lm, const int ref_rank) = 0;

public:
  HCAAbstractClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints);
  ~HCAAbstractClockSync();

  virtual GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c) = 0;
};


#endif /* REPROMPI_BENCH_SYNC_CLOCK_SYNC_SYNC_METHODS_HCAABSTRACTCLOCKSYNC_H_ */
