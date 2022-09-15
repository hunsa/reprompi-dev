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
#include <stdio.h>

#include "dissem_barrier_impl.h"
#include "reprompi_bench/misc.h"

void dissemination_barrier(MPI_Comm comm) {
  int my_rank, np, send_rank, recv_rank;
  int i, nrounds;
  MPI_Status status;
  int send_value = 1;
  int recv_value = 1;

  MPI_Comm_rank(comm, &my_rank);
  MPI_Comm_size(comm, &np);

  nrounds = ceil(log2((double) np));

  for (i = 0; i < nrounds; i++) {
    send_rank = (my_rank + (1<<i)) % np;
    recv_rank = (my_rank - (1 << i) + np) % np;

    //printf("[%d] Sending from %d to %d; receive from %d\n", i, my_rank, send_rank, recv_rank);
    MPI_Sendrecv(&send_value, 1, MPI_INT, send_rank, 0,
                 &recv_value, 1, MPI_INT, recv_rank, 0,
                 comm, &status);
  }

}
