
#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "ClockOffsetAlg.h"


ClockOffsetAlg::ClockOffsetAlg(int rank1, int rank2, MPI_Comm comm) :
  rank1(rank1),
  rank2(rank2),
  comm(comm) {
}

ClockOffsetAlg::~ClockOffsetAlg() {
}

