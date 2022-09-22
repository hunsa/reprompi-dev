/*
 * HCA2ClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <iostream>
//#include <math.h>
//#include <gsl/gsl_fit.h>

#include "ClockPropagationSync.h"
#include "../clocks/GlobalClockLM.h"
#include "../clocks/GlobalClockOffset.h"
#include "../clock_sync_common.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


ClockPropagationSync::ClockPropagationSync() {
}

ClockPropagationSync::~ClockPropagationSync() {
}


GlobalClock* ClockPropagationSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {
  int my_rank, nprocs;
  //int i, j, p;
  GlobalClock *retClock = NULL;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &nprocs);

  GlobalClock *globalClock = dynamic_cast<GlobalClock*>(&c);
  if (globalClock == NULL) {
    return NULL;
  }

  if(nprocs == 1) {
    // only one process, nothing to be done
    return globalClock;
  }

  ZF_LOGV("%d: sync clocks propagation", my_rank);

  if( my_rank == 0 ) {
    int bytes_needed = 0;
    int nested_level = 0;
    bytes_needed = globalClock->get_flattened_clock_size_in_bytes();
    nested_level = globalClock->get_nested_level();
    bytes_needed += sizeof(int); // we prepend how many clocks we will have

    char *buf = new char[bytes_needed];

    ZF_LOGV("%d: flattened clocks have %d bytes", my_rank, bytes_needed);

    globalClock->flatten_clock(buf, (char*)(buf+sizeof(int)), buf+bytes_needed);
    reinterpret_cast<int*>(buf)[0] = nested_level;

    ZF_LOGV("%d: flattening clocks with nested_level=%d done", my_rank, reinterpret_cast<int*>(buf)[0]);

    // source clock
    PMPI_Bcast(&bytes_needed, 1, MPI_INT, 0, comm);
    PMPI_Bcast(buf, bytes_needed, MPI_BYTE, 0, comm);

    // just use my own clock
    retClock = globalClock;

    delete buf;
  } else {
    int bytes_to_receive = 0;
    char *buf;
    Clock *last_clock;

    PMPI_Bcast(&bytes_to_receive, 1, MPI_INT, 0, comm);
    //MPI_Recv(&bytes_to_receive, 1, MPI_INT, 0, 0, comm, &status);
    ZF_LOGV("%d: recv 1 DONE (%d)", my_rank, bytes_to_receive);

    buf = new char[bytes_to_receive];
    PMPI_Bcast(buf, bytes_to_receive, MPI_BYTE, 0, comm);
    //MPI_Recv(buf, bytes_to_receive, MPI_BYTE, 0, 0, comm, &status);
    ZF_LOGV("%d: recv 2 DONE", my_rank);

    int nb_types = reinterpret_cast<int*>(buf)[0];
    ZF_LOGV("%d: nb_types: %d", my_rank, nb_types);

    int current_offset = sizeof(int);
    last_clock = initialize_local_clock();
    for(int i=0; i<nb_types; i++) {
      //int type_id = reinterpret_cast<int*>(buf+current_offset)[0];
      int type_id = reinterpret_cast<int*>(buf+current_offset)[0];
      ZF_LOGV("%d: type_id(%d): %d", my_rank, i, type_id);

      current_offset += sizeof(int);

      switch(type_id) {
      case 0: {
        // GlobalClockLM
        double *data = reinterpret_cast<double*>(buf+current_offset);
        double clock_slope  = data[0];
        double clock_offset = data[1];

        ZF_LOGV("%d: recv glocklm (%g,%g)", my_rank, clock_slope, clock_offset);

        retClock = new GlobalClockLM((Clock&)(*last_clock), clock_slope, clock_offset);
        last_clock = retClock;

        current_offset += 2*sizeof(double);
        break;
      }
      case 1: {
        // GlobalClockOffset
        double *data = reinterpret_cast<double*>(buf+current_offset);
        double clock_offset  = data[0];

        ZF_LOGV("%d: recv glockoff (%g)", my_rank, clock_offset);

        retClock = new GlobalClockOffset((Clock&)(*last_clock), clock_offset);
        last_clock = retClock;

        current_offset += sizeof(double);
        break;
      }
      default: {
        std::cerr << "invalid datatype with id" << type_id << std::endl;
        break;
      }
      }
    }

//    retClock = globalClock->copyClock(c, comm, 0, my_rank);

    delete buf;
  }

  ZF_LOGV("%d: sync clocks propagation END", my_rank);

  return retClock;
}
