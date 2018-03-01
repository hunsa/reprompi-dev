
#include "Clock.h"
#include "GlobalClockOffset.h"


GlobalClockOffset::GlobalClockOffset(Clock* c, double tdiff):
  offset(tdiff),
  GlobalClock(c){
}

GlobalClockOffset::~GlobalClockOffset() {
}


double GlobalClockOffset::apply_clock_model(double timestamp) {
    return timestamp - offset;
}


