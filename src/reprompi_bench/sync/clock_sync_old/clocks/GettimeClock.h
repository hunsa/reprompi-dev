
#ifndef REPROMPIB_MPI_CLOCK_CLASS_H_
#define REPROMPIB_MPI_CLOCK_CLASS_H_

#include "Clock.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

class GettimeClock : public Clock {

public:

  enum class LocalClockType { LOCAL_CLOCK_REALTIME, LOCAL_CLOCK_MONOTONIC };

  GettimeClock(LocalClockType clock_type);
  ~GettimeClock();

  double get_time(void);
  bool is_base_clock();

private:
  double wtime;
  struct timespec ts;
  LocalClockType clock_type;
};




#endif
