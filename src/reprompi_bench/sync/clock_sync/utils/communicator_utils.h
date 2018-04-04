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
#ifndef REPROMPIB_COMMUNICATOR_HELPER_FUNCS_H_
#define REPROMPIB_COMMUNICATOR_HELPER_FUNCS_H_

#include <mpi.h>


void create_intranode_communicator(MPI_Comm old_comm, MPI_Comm *new_comm);

void create_intrasocket_communicator(MPI_Comm old_comm, int socket_id, MPI_Comm *new_comm);

void create_interlevel_communicator(MPI_Comm old_comm, MPI_Comm local_comm,
    const int local_rank_per_node, MPI_Comm *new_comm);

void print_comm_debug_info(char *tag, MPI_Comm comm1, MPI_Comm comm2);

#endif /* REPROMPIB_COMMUNICATOR_HELPER_FUNCS_H_ */

