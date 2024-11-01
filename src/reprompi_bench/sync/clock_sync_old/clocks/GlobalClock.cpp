
#include <iostream>

#include "Clock.h"
#include "GlobalClock.h"

GlobalClock::GlobalClock(Clock& c) :
    local_clock(c) {
}


GlobalClock::~GlobalClock() {
}

double GlobalClock::get_time(void) {
  return apply_clock_model(local_clock.get_time());
}

double GlobalClock::get_local_time(void) {
//  GlobalClock* inner_clock = dynamic_cast<GlobalClock*>(this->local_clock);
//  if (inner_clock != NULL) {
//    return inner_clock->get_local_time();
//  }
  return local_clock.get_time();
}

double GlobalClock::convert_to_global_time(double local_timestamp) {
//  GlobalClock* inner_clock = dynamic_cast<GlobalClock*>(this->local_clock);
//  if (inner_clock  != NULL) {
//    local_timestamp = inner_clock->convert_to_global_time(local_timestamp);
//  }
  double timestamp = local_timestamp;

  if( ! local_clock.is_base_clock() ) {
    GlobalClock& gclock = dynamic_cast<GlobalClock&>(this->local_clock);
    timestamp = gclock.convert_to_global_time(timestamp);
  }

  return apply_clock_model(timestamp);
}

void GlobalClock::print_clock_info() {
  std::cout << "global clock" << std::endl;
}
