
#ifndef REPROMPIB_CLOCK_CLASS_H_
#define REPROMPIB_CLOCK_CLASS_H_

#include <mpi.h>

class Clock {

public:
  Clock() {
  }

  virtual double get_time() {
    return 0;
  }
  virtual ~Clock() {
  }

protected:
  void operator=(Clock const&);

  Clock(Clock &c) {

  }


};




#endif /*  REPROMPIB_CLOCK_CLASS_H_  */
