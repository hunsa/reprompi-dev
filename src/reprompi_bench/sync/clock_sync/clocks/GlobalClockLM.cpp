
#include <iostream>

#include "Clock.h"
#include "GlobalClockLM.h"


//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


GlobalClockLM::GlobalClockLM(Clock &c, double s, double i):
    GlobalClock(c), slope(s), intercept(i)  {
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

bool GlobalClockLM::is_base_clock() {
  return false;
}

void GlobalClockLM::print_clock_info() {
  std::cout << "global clock LM: slope=" << this->slope << " offset=" << this->intercept << std::endl;
}

int GlobalClockLM::get_flattened_clock_size_in_bytes() {
  int ret_nbytes = 0;

  ret_nbytes += this->get_flattened_this_clock_size_in_bytes();

  if( ! local_clock.is_base_clock()) {
    GlobalClock& innerGlobalClock = reinterpret_cast<GlobalClock&>(local_clock);
    ret_nbytes += innerGlobalClock.get_flattened_this_clock_size_in_bytes();
  }

  return ret_nbytes;
}

int GlobalClockLM::get_flattened_this_clock_size_in_bytes()  {
  return 1*sizeof(int)+2*sizeof(double);
}


void GlobalClockLM::flatten_clock(char *buf, char *offset, char* end_pointer) {
  end_pointer -= this->get_flattened_this_clock_size_in_bytes();

  int *data_int= reinterpret_cast<int*>(end_pointer);
  data_int[0] = 0;

  double *data_place = reinterpret_cast<double*>(end_pointer+sizeof(int));
  data_place[0] = this->slope;
  data_place[1] = this->intercept;

  if( ! local_clock.is_base_clock()) {
    // recursevily call flatten_clock
    GlobalClock& innerGlobalClock = reinterpret_cast<GlobalClock&>(local_clock);
    innerGlobalClock.flatten_clock(buf, offset, end_pointer);
  }
}

int GlobalClockLM::get_nested_level() {
  ZF_LOGV("globallm: get_nested_level");
  if( ! local_clock.is_base_clock()) {
    GlobalClock& innerGlobalClock = reinterpret_cast<GlobalClock&>(local_clock);
    return 1 + innerGlobalClock.get_nested_level();
  } else {
    return 1;
  }
}

