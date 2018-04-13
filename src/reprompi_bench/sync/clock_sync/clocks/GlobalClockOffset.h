
#ifndef REPROMPIB_GLOBALCLOCKOFFSET_CLASS_H_
#define REPROMPIB_GLOBALCLOCKOFFSET_CLASS_H_

#include "Clock.h"
#include "GlobalClock.h"

class GlobalClockOffset: public GlobalClock {

private:
  double offset;
  double apply_clock_model(double timestamp);

public:
  GlobalClockOffset(Clock& c, double tdiff = 0);
  ~GlobalClockOffset();
  void print_clock_info();
  GlobalClock* copyClock(Clock &c, MPI_Comm comm, int src_rank, int dst_rank);
  bool is_base_clock();

};


#endif /*  REPROMPIB_GLOBALCLOCKOFFSET_CLASS_H_  */
