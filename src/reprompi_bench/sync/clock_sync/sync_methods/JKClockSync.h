#ifndef REPROMPIB_JKCLOCKSYNC_CLASS_H_
#define REPROMPIB_JKCLOCKSYNC_CLASS_H_

#include <mpi.h>
#include <gsl/gsl_fit.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/sync_methods/ClockSync.h"
#include "reprompi_bench/sync/clock_sync/clock_offset_algs/ClockOffset.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"


template<class OffsetAlgType>
class JKClockSync: public ClockSync {

private:
  int n_fitpoints; /* --fitpoints */
  int n_exchanges; /* --exchanges */
  OffsetAlgType* offset_algs;

public:
  JKClockSync(MPI_Comm comm, Clock* c);
  ~JKClockSync();

  Clock* synchronize_all_clocks(void);

};

template<class OffsetAlgType>
inline JKClockSync<OffsetAlgType>::JKClockSync(MPI_Comm comm, Clock* c) :
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
inline JKClockSync<OffsetAlgType>::~JKClockSync() {
  delete[] offset_algs;
}


template<class OffsetAlgType>
inline Clock* JKClockSync<OffsetAlgType>::synchronize_all_clocks(void) {
  int i, j, p;
   int my_rank, np;
   MPI_Status status;
   int root_rank = 0;
   double slope, intercept;

   slope = 0;
   intercept = 0;

   MPI_Comm_rank(this->comm, &my_rank);
   MPI_Comm_size(this->comm, &np);

   if (my_rank == root_rank) {
       double tlocal, tremote;

       for (j = 0; j < n_fitpoints; j++) {
           for (p = 0; p < np; p++) {
               if (p != root_rank) {
                 ClockOffset* offset = NULL;
                 offset = offset_algs[p].measure_offset(root_rank, *(this->local_clock));
                 delete offset;
               }
           }
       }
   } else {
     double *xfit, *yfit;
     double cov00, cov01, cov11, sumsq;
     int fit;

     xfit = new double[n_fitpoints];
     yfit = new double[n_fitpoints];

     for (j = 0; j < n_fitpoints; j++) {
       ClockOffset* offset = NULL;

       offset = offset_algs[root_rank].measure_offset(root_rank, *(this->local_clock));
       xfit[j] = offset->get_timestamp();
       yfit[j] = offset->get_offset();
       delete offset;
     }

     fit = gsl_fit_linear(xfit, 1, yfit, 1, n_fitpoints, &intercept, &slope, &cov00, &cov01, &cov11, &sumsq);

     delete[] xfit;
     delete[] yfit;
   }
   return new GlobalClockLM(this->local_clock, slope, intercept);
}

#endif /*  REPROMPIB_JKCLOCKSYNC_CLASS_H_  */
