
#include "GettimeClock.h"
#include <time.h>
#include <stdlib.h>

GettimeClock::GettimeClock(LocalClockType clock_type) :
wtime(0.0), clock_type(clock_type)
{
}

GettimeClock::~GettimeClock() {

}

double GettimeClock::get_time(void) {
  if( clock_type == GettimeClock::LocalClockType::CLOCK_REALTIME ) {
    if( clock_gettime( CLOCK_REALTIME, &ts) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }
  } else if( clock_type == GettimeClock::LocalClockType::CLOCK_MONOTONIC ) {
    if( clock_gettime( CLOCK_MONOTONIC, &ts) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }
  }
  wtime = (double)(ts.tv_nsec) / 1.0e+9 + ts.tv_sec;
  return wtime;
}

bool GettimeClock::is_base_clock() {
  return true;
}
