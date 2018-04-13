
#ifndef REPROMPIB_RDTSC_CLOCK_CLASS_H_
#define REPROMPIB_RDTSC_CLOCK_CLASS_H_

#include "reprompi_bench/sync/rdtsc.h"
#include "Clock.h"

class RdtscClock : public Clock {

private:
  double freq_hz;

public:
  RdtscClock() {
    freq_hz = 2300e6;
#ifdef FREQUENCY_MHZ   // set frequency to a fixed value
    freq_hz=FREQUENCY_MHZ*1.0e6;
#endif
  };
  ~RdtscClock() {};


  double get_time(void) {
    return rdtsc()/freq_hz;
  };

  double get_freq(void) {
    return freq_hz;
  };

  bool is_base_clock() {
    return true;
  }

};




#endif /*  REPROMPIB_RDTSC_CLOCK_CLASS_H_  */
