
#ifndef REPROMPIB_RDTSCP_CLOCK_CLASS_H_
#define REPROMPIB_RDTSCP_CLOCK_CLASS_H_

#include "reprompi_bench/sync/rdtsc.h"
#include "Clock.h"

class RdtscpClock : public Clock {

private:
  double freq_hz;

public:
  RdtscpClock() {
    freq_hz = 2300e6;
#ifdef FREQUENCY_MHZ   // set frequency to a fixed value
    freq_hz=FREQUENCY_MHZ*1.0e6;
#endif
  };
  ~RdtscpClock() {};


  double get_time(void) {
    return rdtscp()/freq_hz;
  };

  double get_freq(void) {
    return freq_hz;
  };
};




#endif /*  REPROMPIB_RDTSCP_CLOCK_CLASS_H_  */
