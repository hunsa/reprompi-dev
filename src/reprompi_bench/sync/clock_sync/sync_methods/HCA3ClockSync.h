#ifndef REPROMPIB_HCA3CLOCKSYNC_CLASS_H_
#define REPROMPIB_HCA3CLOCKSYNC_CLASS_H_

#include <mpi.h>
#include <math.h>
#include <gsl/gsl_fit.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"


template<class OffsetAlgType>
class HCA3ClockSync: public ClockSync {

private:
  int n_fitpoints; /* --fitpoints */
  int n_exchanges; /* --exchanges */
  OffsetAlgType* offset_algs;
  lm_t lm;

public:
  HCA3ClockSync(MPI_Comm comm, Clock* c);
  ~HCA3ClockSync();

  lm_t learn_model(const int root_rank, const int other_rank, Clock* current_clock);
  Clock* synchronize_all_clocks(void);

};

template<class OffsetAlgType>
inline HCA3ClockSync<OffsetAlgType>::HCA3ClockSync(MPI_Comm comm, Clock* c) :
    n_fitpoints(1000), n_exchanges(100), ClockSync(comm, c) {
  int nprocs, my_rank, i;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);
  offset_algs = new OffsetAlgType[nprocs];
  for (i = 0; i < nprocs; i++) {
    offset_algs[i] = OffsetAlgType(i, my_rank, comm, n_exchanges);
  }
}

template<class OffsetAlgType>
inline HCA3ClockSync<OffsetAlgType>::~HCA3ClockSync() {
  delete[] offset_algs;
}

template<class OffsetAlgType>
inline lm_t HCA3ClockSync<OffsetAlgType>::learn_model(const int root_rank, const int other_rank, Clock* current_clock) {
  int j;
  int my_rank;
  lm_t lm;

  lm.slope = 0;
  lm.intercept = 0;

  MPI_Comm_rank(this->comm, &my_rank);

  if (my_rank == root_rank) {
    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;
      offset = offset_algs[other_rank].measure_offset(root_rank, *(current_clock));
      delete offset;
    }
  } else if (my_rank == other_rank) {
    double *xfit, *yfit;
    double cov00, cov01, cov11, sumsq;
    int fit;

    xfit = new double[n_fitpoints];
    yfit = new double[n_fitpoints];

    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;

      offset = offset_algs[root_rank].measure_offset(root_rank, *(current_clock));
      xfit[j] = offset->get_timestamp();
      yfit[j] = offset->get_offset();

      delete offset;
    }

    fit = gsl_fit_linear(xfit, 1, yfit, 1, n_fitpoints, &lm.intercept, &lm.slope, &cov00, &cov01, &cov11, &sumsq);
    delete[] xfit;
    delete[] yfit;

  }
  return lm;
}


template<class OffsetAlgType>
inline Clock* HCA3ClockSync<OffsetAlgType>::synchronize_all_clocks(void) {
  int my_rank, nprocs;
  int i;
  int max_power_two, nrounds_step1;
  int running_power, next_power;
  int other_rank;
  lm_t lm;
  Clock* my_clock;

  MPI_Datatype dtype[2] = { MPI_DOUBLE, MPI_DOUBLE };
  int blocklen[2] = { 1, 1 };
  MPI_Aint disp[2] = { 0, sizeof(double) };
  MPI_Datatype mpi_lm_t;
  MPI_Status stat;

  int *step_two_group_ranks;
  int step_two_nb_ranks;
  MPI_Group orig_group, step_two_group;
  MPI_Comm step_two_comm;

  MPI_Comm_rank(this->comm, &my_rank);
  MPI_Comm_size(this->comm, &nprocs);

  nrounds_step1 = floor(log2((double) nprocs));
  max_power_two = (int) pow(2.0, (double) nrounds_step1);

  //MPI_Type_create_struct(2, blocklen, disp, dtype, &mpi_lm_t);
  //MPI_Type_commit(&mpi_lm_t);

  my_clock = new GlobalClockLM(this->local_clock, 0, 0);
  for (i = nrounds_step1; i > 0; i--) {
     running_power = (int) pow(2.0, (double) i);
     next_power = (int) pow(2.0, (double)(i-1));

     if (my_rank >= max_power_two) {
       // nothing to do here
     } else {
       if (my_rank % running_power == 0) { // parent
         other_rank = my_rank + next_power;
         //printf ("[rank=%d] parent=%d child=%d \n", my_rank, my_rank, other_rank);

         // compute model through linear regression
         lm = learn_model(my_rank, other_rank, my_clock);

       } else if (my_rank % running_power == next_power) { // child
         other_rank = my_rank - next_power;
         //printf ("[rank=%d] parent=%d child=%d \n", my_rank, other_rank, my_rank);

         // compute model through linear regression
         lm = learn_model(other_rank, my_rank, my_clock);

         // compute current clock as a global clock based on my drift model with the parent
         my_clock = new GlobalClockLM(this->local_clock, lm.slope, lm.intercept);
       }
     }
     //MPI_Barrier(this->comm);
   }

  // compute the clock models for ranks >= max_power_two (which learn models from ranks 0..(nprocs - max_power_two) in one round)
  if (my_rank >= max_power_two) { // child
    other_rank = my_rank - max_power_two;
    // compute model through linear regression
    lm = learn_model(other_rank, my_rank, my_clock);

    // compute current clock as a global clock based on my drift model with the parent
    my_clock = new GlobalClockLM(this->local_clock, lm.slope, lm.intercept);
  } else if (my_rank < (nprocs - max_power_two)) { // parent
    other_rank = my_rank + max_power_two;

    // compute model through linear regression
    lm = learn_model(my_rank, other_rank, my_clock);
  }

   return my_clock;
}

#endif /*  REPROMPIB_HCA3CLOCKSYNC_CLASS_H_  */
