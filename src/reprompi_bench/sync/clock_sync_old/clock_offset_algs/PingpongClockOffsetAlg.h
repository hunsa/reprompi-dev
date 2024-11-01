
#ifndef REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_

#include "ClockOffsetAlg.h"

#include <map>

class PingpongClockOffsetAlg : public ClockOffsetAlg {

private:
  double rtt;
  int nexchanges_rtt;  // number of exchanges for RTT estimation
  int nexchanges;      // number of ping-ping to be done for clock offset estimation
  std::map<std::string, double> rankpair2rtt;

public:
  PingpongClockOffsetAlg(int nexchanges_rtt, int nexchanges);
  ~PingpongClockOffsetAlg();

  ClockOffset* measure_offset(MPI_Comm comm, int ref_rank, int client_rank, Clock& clock);

};


#endif /*  REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_  */
