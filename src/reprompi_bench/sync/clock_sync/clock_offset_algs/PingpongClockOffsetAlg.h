
#ifndef REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_

#include "ClockOffsetAlg.h"

class PingpongClockOffsetAlg : public ClockOffsetAlg {

private:
  double rtt;

public:
  PingpongClockOffsetAlg();
  ~PingpongClockOffsetAlg();

  ClockOffset* measure_offset(MPI_Comm comm, int ref_rank, int client_rank, int nexchanges, Clock& clock);

};


#endif /*  REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_  */
