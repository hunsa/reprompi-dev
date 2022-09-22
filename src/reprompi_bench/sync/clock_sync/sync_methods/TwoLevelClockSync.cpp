/*
 * TwoLevelClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mpi.h>

#include "TwoLevelClockSync.h"

#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

TwoLevelClockSync::TwoLevelClockSync(ClockSync *syncInterNode, ClockSync *syncIntraNode) :
    syncInterNode(syncInterNode), syncIntraNode(syncIntraNode) {

  this->comm_internode = MPI_COMM_NULL;
  this->comm_intranode = MPI_COMM_NULL;

  this->comm_initialized = false;
}

TwoLevelClockSync::~TwoLevelClockSync() {
  if (comm_internode != MPI_COMM_NULL) {
    MPI_Comm_free(&comm_internode);
  }

  if( comm_intranode != MPI_COMM_NULL ) {
    MPI_Comm_free(&comm_intranode);
  }
}

void TwoLevelClockSync::initialized_communicators(MPI_Comm comm) {
  int my_rank, np;


  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  // create node-level communicators for each node
  create_intranode_communicator(comm, &comm_intranode);

  print_comm_debug_info("intranode", comm, comm_intranode);

  // create an internode-communicator with processes having rank = 0 on the local communicator
  create_interlevel_communicator(comm, comm_intranode, 0, &comm_internode);

  print_comm_debug_info("internode", comm, comm_internode);
}


GlobalClock* TwoLevelClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {

  GlobalClock* global_clock1 = NULL;
  GlobalClock* global_clock2 = NULL;

  int my_rank, np;
  int subcomm_size;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  if( this->comm_initialized == false ) {
    this->initialized_communicators(comm);
    this->comm_initialized = true;
  }

  // Step 1: synchronization between nodes
  if (comm_internode != MPI_COMM_NULL) {
    MPI_Comm_size(comm_internode, &subcomm_size);
    ZF_LOGV("%d: subcomm size:%d", my_rank, subcomm_size);
    if( subcomm_size > 1 ) {
      ZF_LOGV("%d: sync1 real", my_rank);
      global_clock1 = syncInterNode->synchronize_all_clocks(comm_internode, c);
    } else {
      ZF_LOGV("%d: sync1 dummy", my_rank);
      global_clock1 = new GlobalClockLM(c, 0.0, 0.0);
    }
  } else {
    // dummy clock
    ZF_LOGV("%d: sync1 dummy", my_rank);
    global_clock1 = new GlobalClockLM(c, 0.0, 0.0);
  }

#if ZF_LOG_LEVEL == ZF_LOG_VERBOSE
  global_clock1->print_clock_info();
#endif


  ZF_LOGV("%d: sync 2", my_rank);
  // Step 2: synchronization within node
  /*
   * same here
   * if communicator has only one process, then do nothing
   */
  MPI_Comm_size(comm_intranode, &subcomm_size);
  ZF_LOGV("%d: subcomm size:%d", my_rank, subcomm_size);
  if( subcomm_size > 1 ) {
    global_clock2 = syncIntraNode->synchronize_all_clocks(comm_intranode, *(global_clock1));
  } else {
    global_clock2 = global_clock1;
  }

  ZF_LOGV("%d: sync 2 done", my_rank);

  ZF_LOGV("%d: test clock2, %g", my_rank, global_clock2->get_local_time());

  return global_clock2;
}



