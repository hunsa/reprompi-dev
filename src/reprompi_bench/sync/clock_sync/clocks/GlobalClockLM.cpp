

#include "Clock.h"
#include "GlobalClockLM.h"



GlobalClockLM::GlobalClockLM(Clock &c, double s, double i):
  slope(s), intercept(i), GlobalClock(c) {
}

GlobalClockLM::~GlobalClockLM() {
}


double GlobalClockLM::apply_clock_model(double timestamp) {
  return timestamp - (timestamp * this->slope + this->intercept);
}


double GlobalClockLM::get_slope() {
  return this->slope;
}

double GlobalClockLM::get_intercept() {
  return this->intercept;
}

void GlobalClockLM::print_clock_info() {
  printf("global clock LM: slope=%g offset=%g\n", this->slope, this->intercept);
}
