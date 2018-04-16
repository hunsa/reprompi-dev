/*
 * HCA3ClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */


#include <math.h>
#include <gsl/gsl_fit.h>

#include "HCA3ClockSync.h"

#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
//#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


HCA3ClockSync::HCA3ClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints, int n_exchanges) {

//  MPI_Comm_rank(comm, &my_rank);
//  MPI_Comm_size(comm, &nprocs);
  this->offset_alg = offsetAlg;
  this->n_fitpoints = n_fitpoints;
  this->n_exchanges = n_exchanges;
}

HCA3ClockSync::~HCA3ClockSync() {

}

LinModel HCA3ClockSync::learn_model(MPI_Comm comm, const int root_rank, const int other_rank, Clock& current_clock) {
  int j;
  int my_rank;
  LinModel lm;

  lm.slope = 0;
  lm.intercept = 0;

  MPI_Comm_rank(comm, &my_rank);

  ZF_LOGV("%d: learn model %d<->%d", my_rank, root_rank, other_rank);

  if (my_rank == root_rank) {
    for (j = 0; j < n_fitpoints; j++) {
      ClockOffset* offset = NULL;
      offset = offset_alg->measure_offset(comm, root_rank, other_rank, this->n_exchanges, current_clock);
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

      offset = offset_alg->measure_offset(comm, root_rank, my_rank, this->n_exchanges, current_clock);
      xfit[j] = offset->get_timestamp();
      yfit[j] = offset->get_offset();

      delete offset;
    }

    fit = gsl_fit_linear(xfit, 1, yfit, 1, n_fitpoints, &lm.intercept, &lm.slope, &cov00, &cov01, &cov11, &sumsq);
    delete[] xfit;
    delete[] yfit;

  }

  ZF_LOGV("%d: learn model %d<->%d DONE", my_rank, root_rank, other_rank);

  return lm;
}


GlobalClock* HCA3ClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int my_rank, nprocs;
  int i;
  int max_power_two, nrounds_step1;
  int running_power, next_power;
  int other_rank;
  LinModel lm;
  GlobalClock* my_clock;

  MPI_Datatype dtype[2] = { MPI_DOUBLE, MPI_DOUBLE };
  int blocklen[2] = { 1, 1 };
  MPI_Aint disp[2] = { 0, sizeof(double) };
  MPI_Datatype mpi_lm_t;
  MPI_Status stat;

  int *step_two_group_ranks;
  int step_two_nb_ranks;
  MPI_Group orig_group, step_two_group;
  MPI_Comm step_two_comm;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);

  nrounds_step1 = floor(log2((double) nprocs));
  max_power_two = (int) pow(2.0, (double) nrounds_step1);

  ZF_LOGV("%d (%d): nrounds_step1:%d max_power_two:%d", my_rank, nprocs, nrounds_step1, max_power_two);

  //MPI_Type_create_struct(2, blocklen, disp, dtype, &mpi_lm_t);
  //MPI_Type_commit(&mpi_lm_t);

  my_clock = new GlobalClockLM(c, 0, 0);
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

         // compute model through linear regression
         lm = learn_model(comm, my_rank, other_rank, *(my_clock));

       } else if (my_rank % running_power == next_power) { // child
         other_rank = my_rank - next_power;
         ZF_LOGV("[rank=%d] parent=%d child=%d", my_rank, other_rank, my_rank);

         // compute model through linear regression
         lm = learn_model(comm, other_rank, my_rank, *(my_clock));

         // compute current clock as a global clock based on my drift model with the parent
         my_clock = new GlobalClockLM(c, lm.slope, lm.intercept);
       }
     }
     //MPI_Barrier(this->comm);
   }

  // compute the clock models for ranks >= max_power_two (which learn models from ranks 0..(nprocs - max_power_two) in one round)
  if (my_rank >= max_power_two) { // child
    other_rank = my_rank - max_power_two;
    // compute model through linear regression
    lm = learn_model(comm, other_rank, my_rank, *(my_clock));

    // compute current clock as a global clock based on my drift model with the parent
    my_clock = new GlobalClockLM(c, lm.slope, lm.intercept);
  } else if (my_rank < (nprocs - max_power_two)) { // parent
    other_rank = my_rank + max_power_two;

    // compute model through linear regression
    lm = learn_model(comm, my_rank, other_rank, *(my_clock));
  }

   return my_clock;
}



