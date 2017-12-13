#ifndef REPROMPIB_CLOCK_SYNC_UTILS_H_
#define REPROMPIB_CLOCK_SYNC_UTILS_H_

#include <reprompi_bench/sync/clock_sync/clocks/Clock.h>

void compute_rtt(int master_rank, int other_rank, MPI_Comm comm, int nwarmups, int n_pingpongs, Clock& c,  double *rtt);

#endif /* REPROMPIB_CLOCK_SYNC_UTILS_H_ */
