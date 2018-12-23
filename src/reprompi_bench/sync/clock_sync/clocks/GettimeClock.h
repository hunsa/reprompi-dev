
#ifndef REPROMPIB_MPI_CLOCK_CLASS_H_
#define REPROMPIB_MPI_CLOCK_CLASS_H_

#include "Clock.h"

class GettimeClock : public Clock {

public:
  GettimeClock();
  ~GettimeClock();

  double get_time(void);
  bool is_base_clock();

private:
  double wtime;
  struct timespec ts;
};




#endif
