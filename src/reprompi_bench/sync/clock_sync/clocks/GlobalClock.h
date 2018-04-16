#ifndef REPROMPIB_GLOBALCLOCK_CLASS_H_
#define REPROMPIB_GLOBALCLOCK_CLASS_H_

#include "Clock.h"

class GlobalClock: public Clock {

protected:
  Clock& local_clock;
  virtual double apply_clock_model(double timestamp) = 0;     // apply the clock model to a timestamp obtained from my local clock

public:
  GlobalClock(Clock& c);

  double get_local_time(void);
  double get_time(void);                                      // returns global time
  double convert_to_global_time(double local_timestamp);      // converts a local timestamp to a global time
  ~GlobalClock();

  virtual void print_clock_info();
  virtual GlobalClock* copyClock(Clock &c, MPI_Comm comm, int src_rank, int dst_rank) = 0;
  /**
   * this return total nb, including nested clocks
   */
  virtual int get_flattened_clock_size_in_bytes() = 0;
  /**
   * this return total nb, excluding nested clocks
   */
  virtual int get_flattened_this_clock_size_in_bytes() = 0;
  virtual void flatten_clock(char *buf, char *offset, char *end_pointer) = 0;
  virtual int get_nested_level() = 0;
};



#endif /*  REPROMPIB_GLOBALCLOCK_CLASS_H_  */
