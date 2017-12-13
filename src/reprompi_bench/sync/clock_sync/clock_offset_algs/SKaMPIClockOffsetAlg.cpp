#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include <mpi.h>
#include "SKaMPIClockOffsetAlg.h"

SKaMPIClockOffsetAlg::SKaMPIClockOffsetAlg(int ref_rank, int client_rank, MPI_Comm comm, int nexchanges) :
    ping_pong_min_time(-1),
    Minimum_ping_pongs(nexchanges),
    Number_ping_pongs(nexchanges),
    ClockOffsetAlg(ref_rank, client_rank, comm) {
}

SKaMPIClockOffsetAlg::~SKaMPIClockOffsetAlg()
{}

ClockOffset* SKaMPIClockOffsetAlg::measure_offset(int ref_rank, Clock& clock) {  // SKaMPI pingpongs
  int i, other_global_id;
  double s_now, s_last, t_last, t_now;
  double td_min, td_max;
  double invalid_time = -1.0;
  MPI_Status status;
  int pp_tag = 43;
  int p1, p2, my_rank;
  ClockOffset* offset;

  MPI_Comm_rank(this->comm, &my_rank);

  if (my_rank != this->rank1 && my_rank != this->rank2) {
    return NULL;
  }

  if (ref_rank == this->rank1) {
    p1 = this->rank1;
    p2 = this->rank2;
  } else if (ref_rank == this->rank2) {
    p1 = this->rank2;
    p2 = this->rank1;
  } else {
    return NULL;
  }

  /* I had to unroll the main loop because I didn't find a portable way
   to define the initial td_min and td_max with INFINITY and NINFINITY */
  if (my_rank == p1) {

    s_last = clock.get_time();
    MPI_Send(&s_last, 1, MPI_DOUBLE, p2, pp_tag, this->comm);
    MPI_Recv(&t_last, 1, MPI_DOUBLE, p2, pp_tag, this->comm, &status);
    s_now = clock.get_time();
    MPI_Send(&s_now, 1, MPI_DOUBLE, p2, pp_tag, this->comm);

    td_min = t_last - s_now;
    td_max = t_last - s_last;

  } else {
    other_global_id = p1;

    MPI_Recv(&s_last, 1, MPI_DOUBLE, p1, pp_tag, this->comm, &status);
    t_last = clock.get_time();
    MPI_Send(&t_last, 1, MPI_DOUBLE, p1, pp_tag, this->comm);
    MPI_Recv(&s_now, 1, MPI_DOUBLE, p1, pp_tag, this->comm, &status);
    t_now = clock.get_time();

    td_min = s_last - t_last;
    td_min = std::max(td_min, s_now - t_now);

    td_max = s_now - t_last;
  }
  if (my_rank == p1) {
    i = 1;
    while (1) {

      MPI_Recv(&t_last, 1, MPI_DOUBLE, p2, pp_tag, this->comm, &status);
      if (t_last < 0.0) {
        break;
      }

      s_last = s_now;
      s_now = clock.get_time();

      td_min = std::max(td_min, t_last - s_now);
      td_max = std::min(td_max, t_last - s_last);

      if (ping_pong_min_time >= 0.0 && i >= Minimum_ping_pongs && s_now - s_last < ping_pong_min_time * 1.10) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, p2, pp_tag, this->comm);
        break;
      }

      i++;
      if (i == Number_ping_pongs) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, p2, pp_tag, this->comm);
        break;
      }
      MPI_Send(&s_now, 1, MPI_DOUBLE, p2, pp_tag, this->comm);

    }
  } else {
    i = 1;
    while (1) {
      MPI_Send(&t_now, 1, MPI_DOUBLE, p1, pp_tag, this->comm);
      MPI_Recv(&s_last, 1, MPI_DOUBLE, p1, pp_tag, this->comm, &status);
      t_last = t_now;
      t_now = clock.get_time();

      if (s_last < 0.0) {
        break;
      }
      td_min = std::max(td_min, s_last - t_now);
      td_max = std::min(td_max, s_last - t_last);

      if (ping_pong_min_time >= 0.0 && i >= Minimum_ping_pongs && t_now - t_last < ping_pong_min_time * 1.10) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, p1, pp_tag, this->comm);
        break;
      }
      i++;
    }
  }

  if (ping_pong_min_time < 0.0) {
    ping_pong_min_time = td_max - td_min;
  } else {
    ping_pong_min_time = std::min(ping_pong_min_time, td_max - td_min);
  }

  if (my_rank == p1) {
    offset = new ClockOffset(clock.get_time(), (td_min + td_max) / 2.0);
  } else {
    offset = new ClockOffset(clock.get_time(), -(td_min + td_max) / 2.0);
  }

  return offset;
}


