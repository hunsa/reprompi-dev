
#ifndef REPROMPIB_GLOBALCLOCKOFFSET_CLASS_H_
#define REPROMPIB_GLOBALCLOCKOFFSET_CLASS_H_

#include "Clock.h"
#include "GlobalClock.h"

class GlobalClockOffset : public GlobalClock {

private:
  double offset;
  double apply_clock_model(double timestamp);

public:
  GlobalClockOffset(Clock* c = NULL, double tdiff = 0);
  ~GlobalClockOffset();

};


#endif /*  REPROMPIB_GLOBALCLOCKOFFSET_CLASS_H_  */
