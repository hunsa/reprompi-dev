
#ifndef REPROMPIB_RDTSCP_CLOCK_CLASS_H_
#define REPROMPIB_RDTSCP_CLOCK_CLASS_H_

#include "Clock.h"

class RdtscpClock : public Clock {

private:
  double freq_hz;

  __inline__ unsigned long long rdtscp(void) {
      unsigned long long tsc;
      __asm__ __volatile__("rdtscp; "         // serializing read of tsc
              "shl $32,%%rdx; "// shift higher 32 bits stored in rdx up
              "or %%rdx,%%rax"// and or onto rax
              : "=a"(tsc)// output to tsc variable
              :
              : "%rcx", "%rdx");// rcx and rdx are clobbered
      return tsc;
  }


public:
  RdtscpClock() {
    freq_hz = 2300e6;
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
