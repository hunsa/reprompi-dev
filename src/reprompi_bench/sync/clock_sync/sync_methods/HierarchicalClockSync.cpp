/*
 * HierarchicalClockSync.cpp
 *
 *  Created on: Mar 9, 2018
 *      Author: sascha
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mpi.h>

#include "HierarchicalClockSync.h"

#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockOffset.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

HierarchicalClockSync::HierarchicalClockSync(ClockSync *syncInterNode, ClockSync *syncSocket, ClockSync *syncOnSocket) {
  this->syncInterNode = syncInterNode;
  this->syncSocket    = syncSocket;
  this->syncOnSocket  = syncOnSocket;
}

HierarchicalClockSync::~HierarchicalClockSync() {

}


GlobalClock* HierarchicalClockSync::synchronize_all_clocks(MPI_Comm comm, Clock& c) {

  GlobalClock* global_clock1 = NULL;
  GlobalClock* global_clock2 = NULL;
  GlobalClock* global_clock3 = NULL;

  MPI_Comm comm_internode, comm_intranode;
  MPI_Comm comm_intersocket, comm_intrasocket;
  int my_rank, np;
  int socket_id;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  // create node-level communicators for each node
  create_intranode_communicator(comm, &comm_intranode);

  print_comm_debug_info("intranode", comm, comm_intranode);

  // create an internode-communicator with processes having rank = 0 on the local communicator
  create_interlevel_communicator(comm, comm_intranode, 0, &comm_internode);

  print_comm_debug_info("internode", comm, comm_internode);

  // within the node-level communicator, create intra-socket communicators
  socket_id = get_socket_id();
  if (socket_id < 0) {
    // could not get socket information - assign socket 0 to all processes
    if (my_rank == 0) {
      std::cerr << "WARNING: Could not get socket information - assuming all processes run on the same socket\n" << std::endl;
    }
    socket_id = 0;
  }
  create_intrasocket_communicator(comm_intranode, socket_id, &comm_intrasocket);

  print_comm_debug_info("intrasocket", comm, comm_intrasocket);

  // within the node-level communicator, create an inter-socket communicator for each
  // process with rank = 0 on the intra-socket communicator
  create_interlevel_communicator(comm_intranode, comm_intrasocket, 0, &comm_intersocket);

  print_comm_debug_info("intersocket", comm, comm_intersocket);

  // Step 1: synchronization between nodes
  if (comm_internode != MPI_COMM_NULL) {
    ZF_LOGV("%d: sync1 real", my_rank);
    global_clock1 = syncInterNode->synchronize_all_clocks(comm_internode, c);
  } else {
    // dummy clock
    ZF_LOGV("%d: sync1 dummy", my_rank);
    global_clock1 = new GlobalClockLM(c, 0.0, 0.0);
  }

#if ZF_LOG_LEVEL == ZF_LOG_VERBOSE
  global_clock1->print_clock_info();
#endif

  // Step 2: synchronization between sockets
  if (comm_intersocket != MPI_COMM_NULL) {
    ZF_LOGV("%d: sync2 real", my_rank);
    global_clock2 = syncSocket->synchronize_all_clocks(comm_intersocket, *(global_clock1));
    GlobalClockLM *gc1 = dynamic_cast<GlobalClockLM*>(global_clock1);
    GlobalClockLM *gc2 = dynamic_cast<GlobalClockLM*>(global_clock2);
    global_clock2 = new GlobalClockLM(c,
        gc1->get_slope() + gc2->get_slope() - gc1->get_slope() *gc2->get_slope(),
        gc1->get_intercept() + gc2->get_intercept() - gc1->get_slope() * gc2->get_intercept());

  } else {
    ZF_LOGV("%d: sync2 dummy", my_rank);
    global_clock2 = global_clock1;
  }

#if ZF_LOG_LEVEL == ZF_LOG_VERBOSE
  global_clock2->print_clock_info();
#endif

  ZF_LOGV("%d: sync 3", my_rank);

  // Step 3: synchronization within the socket
  // all processes have an intra-socket comm
  global_clock3 = syncOnSocket->synchronize_all_clocks(comm_intrasocket, *(global_clock2));

  ZF_LOGV("sync 3 done");

  if (comm_internode != MPI_COMM_NULL) {
    //printf("[rank %d] has an internode clock\n", my_rank);
    MPI_Comm_free(&comm_internode);

    return global_clock1;
  }

  if (comm_intersocket != MPI_COMM_NULL) {
    //printf("[rank %d] has an intersocket clock\n", my_rank);
    MPI_Comm_free(&comm_intersocket);

    return global_clock2;
  }


  MPI_Comm_free(&comm_intrasocket);
  //printf("[rank %d] has an intrasocket clock\n", my_rank);
  return global_clock3;
}



