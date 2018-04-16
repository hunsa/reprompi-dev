
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
  bool is_base_clock();
  int get_flattened_clock_size_in_bytes();
  int get_flattened_this_clock_size_in_bytes();
  void flatten_clock(char *buf, char *offset, char *end_pointer);
  int get_nested_level();
};


#endif /*  REPROMPIB_GLOBALCLOCKLM_CLASS_H_  */
