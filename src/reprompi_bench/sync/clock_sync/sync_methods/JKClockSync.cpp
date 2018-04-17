/*
 * JKClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <iostream>

#include <mpi.h>
#include <gsl/gsl_fit.h>

#include "JKClockSync.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


JKClockSync::JKClockSync(ClockOffsetAlg *offsetAlg, int n_fitpoints) {
  this->offset_alg = offsetAlg;
  this->n_fitpoints = n_fitpoints;
//  this->n_exchanges = n_exchanges;
}

JKClockSync::~JKClockSync() {
}


GlobalClock* JKClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int i, j, p;
   int my_rank, np;
   MPI_Status status;
   int root_rank = 0;
   double slope, intercept;

   slope = 0;
   intercept = 0;

   MPI_Comm_rank(comm, &my_rank);
   MPI_Comm_size(comm, &np);

   if (my_rank == root_rank) {
       double tlocal, tremote;

       ZF_LOGV("jk:root m offset with %d procs", np);
       for (j = 0; j < this->n_fitpoints; j++) {
           for (p = 0; p < np; p++) {
               if (p != root_rank) {
                 ZF_LOGV("jk:root=%d m offset with %d", my_rank, p);
                 ClockOffset* offset = offset_alg->measure_offset(comm, root_rank, p, c);
                 delete offset;
                 ZF_LOGV("jk:root=%d m offset with %d DONE", my_rank, p);
               }
           }
       }
       ZF_LOGV("jk:root m offset DONE---");
   } else {
     double *xfit, *yfit;
     double cov00, cov01, cov11, sumsq;
     int fit;

     xfit = new double[this->n_fitpoints];
     yfit = new double[this->n_fitpoints];


     for (j = 0; j < this->n_fitpoints; j++) {
       ZF_LOGV("jk:%d m offset with root=%d", my_rank, root_rank);
       ClockOffset* offset = offset_alg->measure_offset(comm, root_rank, my_rank, c);
       xfit[j] = offset->get_timestamp();
       yfit[j] = offset->get_offset();
       delete offset;
       ZF_LOGV("jk:%d m offset with root=%d DONE", my_rank, root_rank);
     }


     fit = gsl_fit_linear(xfit, 1, yfit, 1, this->n_fitpoints, &intercept, &slope, &cov00, &cov01, &cov11, &sumsq);

     delete[] xfit;
     delete[] yfit;
   }
   return new GlobalClockLM(c, slope, intercept);
}
