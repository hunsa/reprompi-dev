
#ifndef REPROMPIB_SKAMPI_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_SKAMPI_CLOCKOFFSETALG_CLASS_H_

#include <mpi.h>
#include "ClockOffsetAlg.h"

class SKaMPIClockOffsetAlg : public ClockOffsetAlg{

private:
  double ping_pong_min_time; /* ping_pong_min_time is the minimum time of one ping_pong
  between the root node and the , negative value means
  time not yet determined;
  needed to avoid measuring again all the 100 RTTs when re-synchronizing
  (in this case only a few ping-pongs are performed if the RTT stays
  within 110% for the ping_pong_min_time)
  */
  int Minimum_ping_pongs;
  int Number_ping_pongs;

public:
  SKaMPIClockOffsetAlg(int ref_rank = 0, int client_rank = 0, MPI_Comm comm = MPI_COMM_NULL, int nexchanges = 0);
  ~SKaMPIClockOffsetAlg();

  ClockOffset* measure_offset(int ref_rank, Clock& clock);
};


#endif /*  REPROMPIB_SKAMPI_CLOCKOFFSETALG_CLASS_H_  */
