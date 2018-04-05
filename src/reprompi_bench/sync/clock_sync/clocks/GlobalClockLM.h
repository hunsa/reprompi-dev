
#ifndef REPROMPIB_GLOBALCLOCKLM_CLASS_H_
#define REPROMPIB_GLOBALCLOCKLM_CLASS_H_

#include "Clock.h"
#include "GlobalClock.h"

class GlobalClockLM : public GlobalClock {

private:
  double slope;
  double intercept;

  double apply_clock_model(double timestamp);

public:
  GlobalClockLM(Clock& c, double s, double i);
  ~GlobalClockLM();

  double get_slope();
  double get_intercept();
  void print_clock_info();
  GlobalClock* copyClock(Clock &c, MPI_Comm comm, int src_rank, int dst_rank);
};


#endif /*  REPROMPIB_GLOBALCLOCKLM_CLASS_H_  */
