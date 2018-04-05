/*
 * HCA2ClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <math.h>
#include <gsl/gsl_fit.h>

#include "ClockPropagationSync.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


ClockPropagationSync::ClockPropagationSync() {
}

ClockPropagationSync::~ClockPropagationSync() {
}


GlobalClock* ClockPropagationSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int my_rank, nprocs;
  int i, j, p;
  GlobalClock *retClock;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);

  GlobalClock *globalClock = dynamic_cast<GlobalClock*>(&c);
  if (globalClock == NULL) {
    return NULL;
  }

  ZF_LOGV("%d: sync clocks propagation", my_rank);

  if( my_rank == 0 ) {
    // source clock
    for(i=1; i<nprocs; i++) {
      globalClock->copyClock(c, comm, 0, i);
    }
    // just use my own clock
    retClock = dynamic_cast<GlobalClock*>(&c);
  } else {
    retClock = globalClock->copyClock(c, comm, 0, my_rank);
  }

  ZF_LOGV("%d: sync clocks propagation END", my_rank);

  return retClock;
}
