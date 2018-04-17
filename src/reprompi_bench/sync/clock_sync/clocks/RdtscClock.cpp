#include "RdtscClock.h"

RdtscClock::RdtscClock() {
  this->freq_hz = 2300e6;
#ifdef FREQUENCY_MHZ   // set frequency to a fixed value
  this->freq_hz=FREQUENCY_MHZ*1.0e6;
#endif
}


RdtscClock::~RdtscClock() {
}

double RdtscClock::get_time(void) {
  return rdtsc() / freq_hz;
}

double RdtscClock::get_freq(void) {
  return freq_hz;
}

bool RdtscClock::is_base_clock() {
  return true;
}

