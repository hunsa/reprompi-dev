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

#ifndef COLLECTIVES_H_
#define COLLECTIVES_H_

#include "reprompi_bench/option_parser/parse_common_options.h"

enum {
    MPI_ALLGATHER = 0,
    MPI_ALLREDUCE,
    MPI_ALLTOALL,
    MPI_BARRIER,
    MPI_BCAST,
    MPI_EXSCAN,
    MPI_GATHER,
    MPI_REDUCE,
    MPI_REDUCE_SCATTER,
    MPI_REDUCE_SCATTER_BLOCK,
    MPI_SCAN,
    MPI_SCATTER,
    PINGPONG_SEND_RECV,
    PINGPONG_SENDRECV,
    PINGPONG_ISEND_RECV,
    PINGPONG_ISEND_IRECV,
    PINGPONG_SEND_IRECV,
    EMPTY,
    N_MPI_CALLS         // number of calls
};


typedef struct collparams {
    size_t count;
    char* sbuf;
    char* rbuf;
    char* tmp_buf;
    int nprocs;
    int root;
    MPI_Datatype datatype;
    MPI_Aint datatype_extent;
    MPI_Op op;
    int rank;
    size_t scount;
    size_t rcount;
    int* counts_array;
    int* displ_array;

    // parameters relevant for ping-pong operations
    int pingpong_ranks[2];
} collective_params_t;


typedef struct basic_collparams {
    int nprocs;
    int root;
    MPI_Datatype datatype;
    MPI_Op op;

    // parameters relevant for ping-pong operations
    int pingpong_ranks[2];
} basic_collective_params_t;



typedef void (*collective_call_t)(collective_params_t* params);
typedef void (*initialize_data_t)(const basic_collective_params_t info, const long count, collective_params_t* params);
typedef void (*cleanup_data_t)(collective_params_t* params);

typedef struct coll_op {
    collective_call_t collective_call;
    initialize_data_t initialize_data;
    cleanup_data_t cleanup_data;
} collective_ops_t;

int get_call_index(char* name);
char* get_call_from_index(int index);
char* const* get_mpi_calls_list(void);

extern const collective_ops_t collective_calls[];

void init_collective_basic_info(reprompib_common_options_t opts, int procs, basic_collective_params_t* coll_basic_info);

void execute_Allgather(collective_params_t* params);
void execute_Allreduce(collective_params_t* params);
void execute_Alltoall(collective_params_t* params);
void execute_Barrier(collective_params_t* params);
void execute_Bcast(collective_params_t* params);
void execute_Exscan(collective_params_t* params);
void execute_Gather(collective_params_t* params);
void execute_Reduce(collective_params_t* params);
void execute_Reduce_scatter(collective_params_t* params);
void execute_Reduce_scatter_block(collective_params_t* params);
void execute_Scan(collective_params_t* params);
void execute_Scatter(collective_params_t* params);

void execute_BBarrier(collective_params_t* params);
void execute_Empty(collective_params_t* params);


// pingpong operations
void execute_pingpong_Send_Recv(collective_params_t* params);
void execute_pingpong_Isend_Recv(collective_params_t* params);
void execute_pingpong_Isend_Irecv(collective_params_t* params);
void execute_pingpong_Send_Irecv(collective_params_t* params);
void execute_pingpong_Sendrecv(collective_params_t* params);



// buffer initialization functions
void initialize_common_data(const basic_collective_params_t info,
        collective_params_t* params);
void initialize_data_default(const basic_collective_params_t info, const long count, collective_params_t* params);

void initialize_data_Allgather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Alltoall(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Bcast(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Gather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Reduce_scatter(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Reduce_scatter_block(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Scatter(const basic_collective_params_t info, const long count, collective_params_t* params);

// buffer initialization for pingpongs
void initialize_data_pingpong(const basic_collective_params_t info, const long count, collective_params_t* params);

// buffer cleanup functions
void cleanup_data_default(collective_params_t* params);

void cleanup_data_Allgather(collective_params_t* params);
void cleanup_data_Alltoall(collective_params_t* params);
void cleanup_data_Bcast(collective_params_t* params);
void cleanup_data_Gather(collective_params_t* params);
void cleanup_data_Reduce_scatter(collective_params_t* params);
void cleanup_data_Reduce_scatter_block(collective_params_t* params);
void cleanup_data_Scatter(collective_params_t* params);

// buffer initialization for pingpongs
void cleanup_data_pingpong(collective_params_t* params);


#endif /* COLLECTIVES_H_ */


