#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>

#include <mpi.h>
#include "SKaMPIClockOffsetAlg.h"

SKaMPIClockOffsetAlg::SKaMPIClockOffsetAlg(int min_n_ping_pong, int n_ping_pongs) {
  this->Minimum_ping_pongs = min_n_ping_pong;
  this->Number_ping_pongs  = n_ping_pongs;
}

SKaMPIClockOffsetAlg::~SKaMPIClockOffsetAlg() {
}

ClockOffset* SKaMPIClockOffsetAlg::measure_offset(MPI_Comm comm, int ref_rank, int client_rank, Clock& clock) {
  // SKaMPI pingpongs
  int i; //, other_global_id;
  double s_now, s_last, t_last, t_now;
  double td_min, td_max;
  double invalid_time = -1.0;
  MPI_Status status;
  int pp_tag = 43;
  //int p1, p2,
  int my_rank;
  ClockOffset* offset;

  double ping_pong_min_time; /* ping_pong_min_time is the minimum time of one ping_pong
   between the root node and the , negative value means
   time not yet determined;
   needed to avoid measuring again all the 100 RTTs when re-synchronizing
   (in this case only a few ping-pongs are performed if the RTT stays
   within 110% for the ping_pong_min_time)
   */

  MPI_Comm_rank(comm, &my_rank);

  // check whether I am participating here
  // if not, there is no offset
  if (my_rank != client_rank && my_rank != ref_rank) {
    return NULL;
  }

  // check whether we have ping_pong_min_time in our hash
  // if so, take it and use it (can stop after min_n_ping_pong rounds)
  // if not, we set ping_pong_min_time to -1.0 (then we need to do n_ping_pongs rounds)
  int rank1;
  int rank2;
  if( ref_rank < client_rank ) {
    rank1 = ref_rank;
    rank2 = client_rank;
  } else {
    rank1 = client_rank;
    rank2 = ref_rank;
  }

  const std::string key_str = std::to_string(rank1) + "," + std::to_string(rank2);

  if( (this->rankpair2minpingpong).find(key_str) != (this->rankpair2minpingpong).end()) {
    ping_pong_min_time = -1.0;
  } else {
    ping_pong_min_time = this->rankpair2minpingpong[key_str];
  }



  /* I had to unroll the main loop because I didn't find a portable way
   to define the initial td_min and td_max with INFINITY and NINFINITY */
  if (my_rank == ref_rank) {

    s_last = clock.get_time();
    MPI_Send(&s_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
    MPI_Recv(&t_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm, &status);
    s_now = clock.get_time();
    MPI_Send(&s_now, 1, MPI_DOUBLE, client_rank, pp_tag, comm);

    td_min = t_last - s_now;
    td_max = t_last - s_last;

  } else {
    //other_global_id = ref_rank;

    MPI_Recv(&s_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
    t_last = clock.get_time();
    MPI_Send(&t_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
    MPI_Recv(&s_now, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
    t_now = clock.get_time();

    td_min = s_last - t_last;
    td_min = std::max(td_min, s_now - t_now);

    td_max = s_now - t_last;
  }
  if (my_rank == ref_rank) {
    i = 1;
    while (1) {

      MPI_Recv(&t_last, 1, MPI_DOUBLE, client_rank, pp_tag, comm, &status);
      if (t_last < 0.0) {
        break;
      }

      s_last = s_now;
      s_now = clock.get_time();

      td_min = std::max(td_min, t_last - s_now);
      td_max = std::min(td_max, t_last - s_last);

      if (ping_pong_min_time >= 0.0 && i >= Minimum_ping_pongs && s_now - s_last < ping_pong_min_time * 1.10) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
        break;
      }

      i++;
      if (i >= Number_ping_pongs) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, client_rank, pp_tag, comm);
        break;
      }
      MPI_Send(&s_now, 1, MPI_DOUBLE, client_rank, pp_tag, comm);

    }
  } else {
    i = 1;
    while (1) {
      MPI_Send(&t_now, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
      MPI_Recv(&s_last, 1, MPI_DOUBLE, ref_rank, pp_tag, comm, &status);
      t_last = t_now;
      t_now = clock.get_time();

      if (s_last < 0.0) {
        break;
      }
      td_min = std::max(td_min, s_last - t_now);
      td_max = std::min(td_max, s_last - t_last);

      if (ping_pong_min_time >= 0.0 && i >= Minimum_ping_pongs && t_now - t_last < ping_pong_min_time * 1.10) {
        MPI_Send(&invalid_time, 1, MPI_DOUBLE, ref_rank, pp_tag, comm);
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

  // update pingpong hash
  this->rankpair2minpingpong[key_str] = ping_pong_min_time;

  if (my_rank == ref_rank) {
    offset = new ClockOffset(clock.get_time(), (td_min + td_max) / 2.0);
  } else {
    offset = new ClockOffset(clock.get_time(), -(td_min + td_max) / 2.0);
  }

  return offset;
}


