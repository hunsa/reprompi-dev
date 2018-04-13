
#ifndef REPROMPIB_MPI_CLOCK_CLASS_H_
#define REPROMPIB_MPI_CLOCK_CLASS_H_

#include <mpi.h>
#include "Clock.h"

class MPIClock : public Clock {

public:
  MPIClock() {};
  ~MPIClock() {};

  double get_time(void) {
    return MPI_Wtime();
  };

  bool is_base_clock() {
    return true;
  }

};




#endif /*  REPROMPIB_MPI_CLOCK_CLASS_H_  */
