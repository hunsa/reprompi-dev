
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

  /* is this one a base clock?
   * others are just derived (composed, decorator pattern)
   */
  virtual bool is_base_clock() = 0;

protected:
  void operator=(Clock const&);

  Clock(Clock &c) {

  }


};




#endif /*  REPROMPIB_CLOCK_CLASS_H_  */
