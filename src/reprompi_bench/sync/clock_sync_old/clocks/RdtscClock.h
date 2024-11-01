
#ifndef REPROMPIB_RDTSC_CLOCK_CLASS_H_
#define REPROMPIB_RDTSC_CLOCK_CLASS_H_

#include "reprompi_bench/sync/rdtsc.h"
#include "Clock.h"

class RdtscClock : public Clock {

private:
  double freq_hz;

public:
  RdtscClock();
  ~RdtscClock();

  double get_time(void);
  double get_freq(void);
  bool is_base_clock();

};




#endif /*  REPROMPIB_RDTSC_CLOCK_CLASS_H_  */
