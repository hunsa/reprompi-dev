#include <iostream>

#include "Clock.h"
#include "GlobalClockOffset.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

GlobalClockOffset::GlobalClockOffset(Clock& c, double tdiff) :
    GlobalClock(c), offset(tdiff) {
}

GlobalClockOffset::~GlobalClockOffset() {
}


double GlobalClockOffset::apply_clock_model(double timestamp) {
    return timestamp - offset;
}

bool GlobalClockOffset::is_base_clock() {
  return false;
}

void GlobalClockOffset::print_clock_info() {
  std::cout << "global clock OFFSET: offset=" << this->offset << std::endl;
}

int GlobalClockOffset::get_flattened_clock_size_in_bytes() {
  int ret_nbytes = 0;

  ret_nbytes += this->get_flattened_this_clock_size_in_bytes();

  if( ! local_clock.is_base_clock()) {
    GlobalClock& innerGlobalClock = reinterpret_cast<GlobalClock&>(local_clock);
    ret_nbytes += innerGlobalClock.get_flattened_this_clock_size_in_bytes();
  }

  return ret_nbytes;
}

int GlobalClockOffset::get_flattened_this_clock_size_in_bytes() {
  // type (GlobalClockOffset int = 0)
  // actual clock offset (double)
  return sizeof(int)+sizeof(double);
}

void GlobalClockOffset::flatten_clock(char *buf, char *offset, char* end_pointer) {
  end_pointer -= this->get_flattened_clock_size_in_bytes();

  int *data_int= reinterpret_cast<int*>(end_pointer);
  data_int[0] = 0;

  double *data_place = reinterpret_cast<double*>(end_pointer+sizeof(int));
  data_place[0] = this->offset;

  if( ! local_clock.is_base_clock()) {
    // recursevily call flatten_clock
    GlobalClock& innerGlobalClock = reinterpret_cast<GlobalClock&>(local_clock);
    innerGlobalClock.flatten_clock(buf, offset, end_pointer);
  }
}

int GlobalClockOffset::get_nested_level() {
  if( ! local_clock.is_base_clock()) {
    GlobalClock& innerGlobalClock = reinterpret_cast<GlobalClock&>(local_clock);
    return 1 + innerGlobalClock.get_nested_level();
  } else {
    return 1;
  }
}
