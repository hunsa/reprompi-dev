
#include "HCA3OffsetClockSync.h"
#include <math.h>

#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"
//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


HCA3OffsetClockSync::HCA3OffsetClockSync(ClockOffsetAlg *offsetAlg) :
offset_alg(offsetAlg)
{
}

HCA3OffsetClockSync::~HCA3OffsetClockSync() {
  delete offset_alg;
}


GlobalClock* HCA3OffsetClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int my_rank, nprocs;
  int i;
  int max_power_two, nrounds_step1;
  int running_power, next_power;
  int other_rank;
  GlobalClock* my_clock;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);

  nrounds_step1 = floor(log2((double) nprocs));
  max_power_two = (int) pow(2.0, (double) nrounds_step1);

  ZF_LOGV("%d (%d): nrounds_step1:%d max_power_two:%d", my_rank, nprocs, nrounds_step1, max_power_two);

  //MPI_Type_create_struct(2, blocklen, disp, dtype, &mpi_lm_t);
  //MPI_Type_commit(&mpi_lm_t);

  my_clock = new GlobalClockOffset(c, 0.0);
  for (i = nrounds_step1; i > 0; i--) {
    running_power = (int) pow(2.0, (double) i);
    next_power = (int) pow(2.0, (double)(i-1));

    ZF_LOGV("%d: running_power:%d next_power:%d", my_rank, running_power, next_power);

    if (my_rank >= max_power_two) {
      // nothing to do here
    } else {
      if (my_rank % running_power == 0) { // parent
        other_rank = my_rank + next_power;
        ZF_LOGV("[rank=%d] parent=%d child=%d", my_rank, my_rank, other_rank);

        ClockOffset* co = offset_alg->measure_offset(comm, my_rank, other_rank, *(my_clock));
        delete co;
//        // compute model through linear regression
//        lm = learn_model(comm, *(my_clock), my_rank, other_rank);

      } else if (my_rank % running_power == next_power) { // child
        other_rank = my_rank - next_power;
        ZF_LOGV("[rank=%d] parent=%d child=%d", my_rank, other_rank, my_rank);

//        // compute model through linear regression
//        lm = learn_model(comm, *(my_clock), other_rank, my_rank);
        ClockOffset* co = offset_alg->measure_offset(comm, other_rank, my_rank, *(my_clock));

        // compute current clock as a global clock based on my drift model with the parent
        my_clock = new GlobalClockOffset(c, co->get_offset());
        delete co;

      }
    }
    //MPI_Barrier(this->comm);
  }

  // compute the clock models for ranks >= max_power_two (which learn models from ranks 0..(nprocs - max_power_two) in one round)
  if (my_rank >= max_power_two) { // child
    other_rank = my_rank - max_power_two;

//    // compute model through linear regression
//    lm = learn_model(comm, *(my_clock), other_rank, my_rank);

    ClockOffset* co = offset_alg->measure_offset(comm, other_rank, my_rank, *(my_clock));
    // compute current clock as a global clock based on my drift model with the parent
    my_clock = new GlobalClockOffset(c, co->get_offset());
    delete co;

  } else if (my_rank < (nprocs - max_power_two)) { // parent
    other_rank = my_rank + max_power_two;

//    // compute model through linear regression
//    lm = learn_model(comm, *(my_clock), my_rank, other_rank);
    ClockOffset* co = offset_alg->measure_offset(comm, my_rank, other_rank, *(my_clock));
    delete co;
  }

  return my_clock;
}

GlobalClock* HCA3OffsetClockSync::create_global_dummy_clock(MPI_Comm comm, Clock& c) {
  return new GlobalClockOffset(c, 0.0);
}


