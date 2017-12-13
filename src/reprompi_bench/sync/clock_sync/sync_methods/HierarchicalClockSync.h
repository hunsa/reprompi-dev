
#ifndef REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_
#define REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_
#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include "reprompi_bench/sync/clock_sync/clocks/Clock.h"
#include "reprompi_bench/sync/clock_sync/clocks/GlobalClockLM.h"
#include "reprompi_bench/sync/clock_sync/utils/communicator_utils.h"
#include "ClockSync.h"

#include "reprompi_bench/sync/clock_sync/utils/hwloc_helpers.h"

template<class SyncIN, class SyncIS, class SyncIntra>//, class DefaultClock>
class HierarchicalClockSync : public ClockSync {

public:
   HierarchicalClockSync(MPI_Comm comm, Clock* c); //, SyncConfiguration& conf);
   ~HierarchicalClockSync();

  Clock* synchronize_all_clocks(void);

};



template<class SyncIN, class SyncIS, class SyncIntra>
inline HierarchicalClockSync<SyncIN, SyncIS, SyncIntra>::HierarchicalClockSync(MPI_Comm comm,
    Clock* c):
    ClockSync(comm, c)
    //conf(conf)
{}

template<class SyncIN, class SyncIS, class SyncIntra>
inline HierarchicalClockSync<SyncIN, SyncIS, SyncIntra>::~HierarchicalClockSync()
{}


template<class SyncIN, class SyncIS, class SyncIntra>
inline Clock* HierarchicalClockSync<SyncIN, SyncIS, SyncIntra>::synchronize_all_clocks(void) {

  Clock* global_clock1 = NULL;
  Clock* global_clock2 = NULL;
  Clock* global_clock3 = NULL;
  SyncIN* internode_clocksync = NULL;
  SyncIS* intersocket_clocksync = NULL;
  SyncIntra* intrasocket_clocksync = NULL;

  MPI_Comm comm_internode, comm_intranode;
  MPI_Comm comm_intersocket, comm_intrasocket;
  int my_rank, np;
  int socket_id;

  MPI_Comm_rank(this->comm, &my_rank);
  MPI_Comm_size(this->comm, &np);

  // create node-level communicators for each node
  create_intranode_communicator(this->comm, &comm_intranode);

  // create an internode-communicator with processes having rank = 0 on the local communicator
  create_interlevel_communicator(this->comm, comm_intranode, 0, &comm_internode);

  // within the node-level communicator, create intra-socket communicators
  socket_id = get_socket_id();
  create_intrasocket_communicator(comm_intranode, socket_id, &comm_intrasocket);

  // within the node-level communicator, create an inter-socket communicator for each
  // process with rank = 0 on the intra-socket communicator
  create_interlevel_communicator(comm_intranode, comm_intrasocket, 0, &comm_intersocket);

  // Step 1: synchronization between nodes
  if (comm_internode != MPI_COMM_NULL) {
    internode_clocksync = new SyncIN(comm_internode, this->local_clock);

    global_clock1 = internode_clocksync->synchronize_all_clocks();
  } else {
    global_clock1 = this->local_clock;
  }

  // Step 2: synchronization between sockets
  if (comm_intersocket != MPI_COMM_NULL) {
      intersocket_clocksync = new SyncIS(comm_intersocket, global_clock1);
      global_clock2 = intersocket_clocksync->synchronize_all_clocks();
  } else {
      global_clock2 = global_clock1;
  }

  // Step 3: synchronization within the socket
  // all processes have an intra-socket comm
  intrasocket_clocksync = new SyncIntra(comm_intrasocket, global_clock2);
  global_clock3 = intrasocket_clocksync->synchronize_all_clocks();

  delete internode_clocksync;
  delete intersocket_clocksync;
  delete intrasocket_clocksync;

  if (comm_internode != MPI_COMM_NULL) {
    //printf("[rank %d] has an internode clock\n", my_rank);
    MPI_Comm_free(&comm_internode);
    delete global_clock2;
    delete global_clock3;

    return global_clock1;
  }
  if (comm_intersocket != MPI_COMM_NULL) {
    //printf("[rank %d] has an intersocket clock\n", my_rank);
    MPI_Comm_free(&comm_intersocket);
    delete global_clock3;

    return global_clock2;
  }

  MPI_Comm_free(&comm_intrasocket);
  //printf("[rank %d] has an intrasocket clock\n", my_rank);
  return global_clock3;
}

#endif /*  REPROMPIB_HIERARCHICALCLOCKSYNC_CLASS_H_  */
