
#ifndef REPROMPIB_RDTSC_CLOCK_CLASS_H_
#define REPROMPIB_RDTSC_CLOCK_CLASS_H_

#include "Clock.h"

class RdtscClock : public Clock {

private:
  double freq_hz;

  __inline__ unsigned long long rdtsc(void)
  {
      unsigned hi, lo;
      __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
      return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
  }

public:
  RdtscClock() {
    freq_hz = 2300e6;
  };
  ~RdtscClock() {};


  double get_time(void) {
    return rdtsc()/freq_hz;
  };

  double get_freq(void) {
    return freq_hz;
  };
};




#endif /*  REPROMPIB_RDTSC_CLOCK_CLASS_H_  */
