
#ifndef REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_

#include "ClockOffsetAlg.h"

class PingpongClockOffsetAlg : public ClockOffsetAlg {

private:
  int nexchanges;
  double rtt;

public:
  PingpongClockOffsetAlg(int rank1 = 0, int rank2 = 0, MPI_Comm comm = MPI_COMM_NULL, int nexchanges = 0);
  ~PingpongClockOffsetAlg();

  ClockOffset* measure_offset(int ref_rank, Clock& clock);

};


#endif /*  REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_  */
