

#include "Clock.h"
#include "GlobalClockLM.h"



GlobalClockLM::GlobalClockLM(Clock* c, double s, double i):
  slope(s), intercept(i), GlobalClock(c) {
}

GlobalClockLM::~GlobalClockLM() {
}


double GlobalClockLM::apply_clock_model(double timestamp) {
  return timestamp - (timestamp * slope + intercept);
}


double GlobalClockLM::get_slope() {
  return slope;
}

double GlobalClockLM::get_intercept() {
  return intercept;
}

