#include <iostream>

#include "Clock.h"
#include "GlobalClockOffset.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"

GlobalClockOffset::GlobalClockOffset(Clock& c, double tdiff) :
    offset(tdiff), GlobalClock(c) {
}

GlobalClockOffset::~GlobalClockOffset() {
}


double GlobalClockOffset::apply_clock_model(double timestamp) {
    return timestamp - offset;
}

bool GlobalClockOffset::is_base_clock() {
  return false;
}

void GlobalClockOffset::print_clock_info() {
  std::cout << "global clock OFFSET: offset=" << this->offset << std::endl;
}

GlobalClock* GlobalClockOffset::copyClock(Clock &c, MPI_Comm comm, int src_rank, int dst_rank)  {
  int my_rank;
  GlobalClock *retClock;

  MPI_Comm_rank(comm, &my_rank);

  ZF_LOGV("%d: copy clock OFFSET START", my_rank);

  if( my_rank == src_rank ) {
    double msg[] = { this->offset };
    MPI_Send(msg, 2, MPI_DOUBLE, dst_rank, 0, comm);
  } else if( my_rank == dst_rank ){
    double msg[1];
    MPI_Status status;
    MPI_Recv(msg, 1, MPI_DOUBLE, src_rank, 0, comm, &status);
    retClock = new GlobalClockOffset(c, msg[0]);
  } else {
    retClock = NULL;
  }

  return retClock;
}


