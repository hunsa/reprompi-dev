/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
 */

#include <stdlib.h>
#include <mpi.h>

#define ALLREDUCE_TAG 777

static int Allreduce_bitwise_commutative(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op,
    MPI_Comm comm);
static int RemproMPI_Bcast_binomial(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);


inline unsigned int flp2(unsigned int n) {
  n = (n | n >> 1);
  n = (n | n >> 2);
  n = (n | n >> 4);
  n = (n | n >> 8);
  n = (n | n >> 16);

  return n - (n >> 1);
}

// Algorithm of Bar-Noy, Kipnis, Schieber, 1993; Bruck, Ho, 1993
int Allreduce_bitwise_commutative(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op,
    MPI_Comm comm) {
  int rank, size;
  int cyci, cyco; // cyclic (circulant) in, out ranks
  int c0, c1, n;
  int c, cc; // simplification
  int bit;
  int b;
  int s; // checking: total number of blocks received

//  int commute;

  MPI_Aint lb, extent;

  void *partbuf, *tempbuf, *inbuf, *outbuf;

//  MPI_Op_commutative(op, &commute);
//  assert(commute);

  MPI_Comm_rank(comm,&rank);
  MPI_Comm_size(comm,&size);

  MPI_Type_get_extent(datatype, &lb, &extent);

  if (sendbuf!=MPI_IN_PLACE) {
    MPI_Sendrecv(sendbuf,count,datatype,0,ALLREDUCE_TAG,
     recvbuf,count,datatype,0,ALLREDUCE_TAG,
     MPI_COMM_SELF,MPI_STATUS_IGNORE);
  }

  partbuf = (void*) malloc(count * extent);
  inbuf = (void*) malloc(count * extent);

  n = size-1;
  bit = flp2(n);

  c0 = 0;
  c1 = 1;
  c = 0;

  tempbuf = partbuf;

  s = 1;
  while (bit!=0x0) {
    //void *buf;
    if ((n&bit)==bit) {
      cyci = (rank+c1)%size;
      cyco = (rank-c1+size)%size;
      b = c1;
      outbuf = recvbuf;
      cc = c+1;
    } else {
      cyci = (rank+c0)%size;
      cyco = (rank-c0+size)%size;
      b = c0;
      outbuf = partbuf;
      cc = c;
    }

    //if (rank==0) fprintf(stderr,"bit %d c0 %d c1 %d b %d c %d cc %d\n",bit,c0,c1,b,c,cc);

    MPI_Sendrecv(outbuf, count, datatype, cyco, ALLREDUCE_TAG, tempbuf, count, datatype, cyci, ALLREDUCE_TAG, comm,
        MPI_STATUS_IGNORE);
    s += b;

    MPI_Reduce_local(tempbuf, recvbuf, count, datatype, op);
    if (tempbuf != partbuf) {
      MPI_Reduce_local(tempbuf, partbuf, count, datatype, op);
    }
    tempbuf = inbuf;

    c = (c<<1);
    if ((n&bit)==bit) {
      c0 = c0+c1;
      c1 = c1+c1;
      c++;
    } else {
      c1 = c1+c0;
      c0 = c0+c0;
    }

    bit >>= 1;
  }
  //assert(s==size);

  free(partbuf);
  free(inbuf);

  return MPI_SUCCESS;
}

int RemproMPI_Bcast_binomial(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
  int rank, size;
  int d, dd = 0;

  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  d = 1;
  dd = 0;
  while (rank >= d) {
    dd = d;
    d <<= 1;
  }

  if (rank != root) {
    MPI_Recv(buffer, count, datatype, rank - dd, 0, comm, MPI_STATUS_IGNORE);
  }

  while (rank + d < size) {
    MPI_Send(buffer, count, datatype, rank + d, 0, comm);
    d <<= 1;
  }

  return MPI_SUCCESS;
}


int ReproMPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
  return Allreduce_bitwise_commutative(sendbuf, recvbuf, count, datatype, op, comm);
}


int ReproMPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
  return RemproMPI_Bcast_binomial(buffer, count, datatype, root, comm);
}
