#include "RdtscpClock.h"

RdtscpClock::RdtscpClock() {
  freq_hz = 2300e6;
#ifdef FREQUENCY_MHZ   // set frequency to a fixed value
  freq_hz=FREQUENCY_MHZ*1.0e6;
#endif
}

RdtscpClock::~RdtscpClock() {
}

double RdtscpClock::get_time(void) {
  return rdtscp() / freq_hz;
}

double RdtscpClock::get_freq(void) {
  return freq_hz;
}

bool RdtscpClock::is_base_clock() {
  return true;
}

