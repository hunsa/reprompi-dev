
#ifndef REPROMPIB_SKAMPI_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_SKAMPI_CLOCKOFFSETALG_CLASS_H_

#include <map>
#include <mpi.h>
#include "ClockOffsetAlg.h"

class SKaMPIClockOffsetAlg: public ClockOffsetAlg {

private:
  int Minimum_ping_pongs;
  int Number_ping_pongs;
  std::map<std::string, double> rankpair2minpingpong;

public:
  SKaMPIClockOffsetAlg(int min_n_ping_pong, int n_ping_pongs);
  ~SKaMPIClockOffsetAlg();

  ClockOffset* measure_offset(MPI_Comm comm, int ref_rank, int client_rank, Clock& clock);
};

#endif /*  REPROMPIB_SKAMPI_CLOCKOFFSETALG_CLASS_H_  */
