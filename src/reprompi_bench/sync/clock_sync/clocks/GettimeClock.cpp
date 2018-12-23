
#include "GettimeClock.h"
#include <time.h>
#include <stdlib.h>

GettimeClock::GettimeClock() {
  wtime = 0.0;
}

GettimeClock::~GettimeClock() {

}

double GettimeClock::get_time(void) {
  if( clock_gettime( CLOCK_REALTIME, &ts) == -1 ) {
    perror( "clock gettime" );
    exit( EXIT_FAILURE );
  }
  wtime = (double)(ts.tv_nsec) / 1.0e+9 + ts.tv_sec;
  return wtime;
}

bool GettimeClock::is_base_clock() {
  return true;
}
