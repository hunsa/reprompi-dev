
#ifndef REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_
#define REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_

#include "ClockOffsetAlg.h"

#include<map>
#include<tuple>

//typedef std::tuple<int,int> tuple_t2;

class PingpongClockOffsetAlg : public ClockOffsetAlg {

private:
  double rtt;
  //std::map<tuple_t2, double> rtt_map;

public:
  PingpongClockOffsetAlg();
  ~PingpongClockOffsetAlg();

  ClockOffset* measure_offset(MPI_Comm comm, int ref_rank, int client_rank, int nexchanges, Clock& clock);

};


#endif /*  REPROMPIB_PINGPONG_CLOCKOFFSETALG_CLASS_H_  */
