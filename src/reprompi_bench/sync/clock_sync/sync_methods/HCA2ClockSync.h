#ifndef REPROMPIB_HCACLOCKSYNC_CLASS_H_
#define REPROMPIB_HCACLOCKSYNC_CLASS_H_

#include <mpi.h>

#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffsetAlg.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "ClockSync.h"


class HCA2ClockSync: public ClockSync {     // Hierarchical synchronization in log2(p) steps

private:
  int n_fitpoints; /* --fitpoints */
  int n_exchanges; /* --exchanges */
  ClockOffsetAlg *offset_alg;

  LinModel learn_model(MPI_Comm comm, Clock &c, const int root_rank, const int other_rank);
  LinModel merge_linear_models(LinModel lm1, LinModel lm2);

protected:
  int my_pow_2(int exp);


  virtual void compute_and_set_all_intercepts(LinModel* lm);
  virtual void compute_and_set_intercept(LinModel* lm, int client, int p_ref);

public:
  HCA2ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, int n_exchanges);
  ~HCA2ClockSync();

  GlobalClock* synchronize_all_clocks(MPI_Comm comm, Clock& c);

};

#endif /*  REPROMPIB_HCACLOCKSYNC_CLASS_H_  */
