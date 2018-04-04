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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>


void create_intranode_communicator(MPI_Comm old_comm, MPI_Comm *new_comm) {
  int my_rank;

  MPI_Comm_rank(old_comm, &my_rank);
  MPI_Comm_split_type(old_comm, MPI_COMM_TYPE_SHARED, my_rank, MPI_INFO_NULL, new_comm);
}

void create_intrasocket_communicator(MPI_Comm old_comm, int socket_id, MPI_Comm *new_comm) {
  int my_rank, local_nprocs;
  int local_rank;
  char procname[MPI_MAX_PROCESSOR_NAME];
  int len;

  MPI_Comm_rank(old_comm, &my_rank);
  MPI_Comm_split(old_comm, socket_id, my_rank, new_comm);
}


void create_interlevel_communicator(MPI_Comm old_comm, MPI_Comm local_comm,
    const int local_rank_per_node, MPI_Comm *new_comm) {
  int my_rank, local_nprocs;
  int local_rank;
  char procname[MPI_MAX_PROCESSOR_NAME];
  int len;

  MPI_Comm_rank(old_comm, &my_rank);
  MPI_Comm_rank(local_comm,&local_rank);
  MPI_Comm_size(local_comm,&local_nprocs);

  if (local_rank_per_node >= local_nprocs) {
    MPI_Get_processor_name(procname, &len);
    fprintf(stderr, "ERROR: Not enough processes allocated to %s\n", procname);
    exit(1);
  }

  if (local_rank == local_rank_per_node) {  // create a communicator including one process per node
    MPI_Comm_split(old_comm, local_rank, my_rank, new_comm);
  } else {  // do not create communicators for other processes
    MPI_Comm_split(old_comm, MPI_UNDEFINED, my_rank, new_comm);
  }
}

void print_comm_debug_info(char *tag, MPI_Comm comm1, MPI_Comm comm2) {
  int rank1 = -1, rank2 = -1;
  int size1 = -1, size2 = -1;

  if( comm1 != MPI_COMM_NULL) {
    MPI_Comm_rank(comm1, &rank1);
    MPI_Comm_size(comm1, &size1);
  }

  if( comm2 != MPI_COMM_NULL) {
    MPI_Comm_rank(comm2, &rank2);
    MPI_Comm_size(comm2, &size2);
  }

  printf("%s: comm1: rank=%d (%d), comm2: rank=%d (%d)\n", tag, rank1, size1, rank2, size2);

}

