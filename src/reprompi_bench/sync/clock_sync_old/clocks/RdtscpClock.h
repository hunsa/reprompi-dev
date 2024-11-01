
#ifndef REPROMPIB_RDTSCP_CLOCK_CLASS_H_
#define REPROMPIB_RDTSCP_CLOCK_CLASS_H_

#include "reprompi_bench/sync/rdtsc.h"
#include "Clock.h"

class RdtscpClock : public Clock {

private:
  double freq_hz;

public:
  RdtscpClock();
  ~RdtscpClock();

  double get_time(void);
  double get_freq(void);
  bool is_base_clock();

};




#endif /*  REPROMPIB_RDTSCP_CLOCK_CLASS_H_  */
