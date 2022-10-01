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
#include <assert.h>
#include <limits.h>
#include <mpi.h>
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "buf_manager/mem_allocation.h"
#include "reprompi_bench/misc.h"
#include "collectives.h"

const collective_ops_t collective_calls[] = {
        [MPI_ALLGATHER] = {
                &execute_Allgather,
                &initialize_data_Allgather,
                &cleanup_data_Allgather
        },
        [MPI_ALLREDUCE] = {
                &execute_Allreduce,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_ALLTOALL] = {
                &execute_Alltoall,
                &initialize_data_Alltoall,
                &cleanup_data_Alltoall
        },
        [MPI_BARRIER] = {
                &execute_Barrier,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_BCAST] =  {
                &execute_Bcast,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_EXSCAN] = {
                &execute_Exscan,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_GATHER] = {
                &execute_Gather,
                &initialize_data_Gather,
                &cleanup_data_Gather
        },
        [MPI_REDUCE] = {
                &execute_Reduce,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_REDUCE_SCATTER] = {
                &execute_Reduce_scatter,
                &initialize_data_Reduce_scatter,
                &cleanup_data_Reduce_scatter
        },
        [MPI_REDUCE_SCATTER_BLOCK] = {
                &execute_Reduce_scatter_block,
                &initialize_data_Reduce_scatter_block,
                &cleanup_data_Reduce_scatter_block
        },
        [MPI_SCAN] = {
                &execute_Scan,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_SCATTER] = {
                &execute_Scatter,
                &initialize_data_Scatter,
                &cleanup_data_Scatter
        },
        [PINGPONG_SEND_RECV] = {
                &execute_pingpong_Send_Recv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_SENDRECV] = {
                &execute_pingpong_Sendrecv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_ISEND_RECV] = {
                &execute_pingpong_Isend_Recv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_ISEND_IRECV] = {
                &execute_pingpong_Isend_Irecv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_SEND_IRECV] = {
                &execute_pingpong_Send_Irecv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [EMPTY] = {
                &execute_Empty,
                &initialize_data_default,
                &cleanup_data_default
        }
};


static char* const mpi_calls_opts[] = {
        [MPI_ALLGATHER] = "MPI_Allgather",
        [MPI_ALLREDUCE] = "MPI_Allreduce",
        [MPI_ALLTOALL] = "MPI_Alltoall",
        [MPI_BARRIER] = "MPI_Barrier",
        [MPI_BCAST] =  "MPI_Bcast",
        [MPI_EXSCAN] = "MPI_Exscan",
        [MPI_GATHER] = "MPI_Gather",
        [MPI_REDUCE] = "MPI_Reduce",
        [MPI_REDUCE_SCATTER] = "MPI_Reduce_scatter",
        [MPI_REDUCE_SCATTER_BLOCK] = "MPI_Reduce_scatter_block",
        [MPI_SCAN] = "MPI_Scan",
        [MPI_SCATTER] = "MPI_Scatter",
        [PINGPONG_SEND_RECV] = "Send_Recv",
        [PINGPONG_SENDRECV] = "Sendrecv",
        [PINGPONG_ISEND_RECV] = "Isend_Recv",
        [PINGPONG_ISEND_IRECV] = "Isend_Irecv",
        [PINGPONG_SEND_IRECV] = "Send_Irecv",
        [EMPTY] = "Empty",
        NULL
};

char* const* get_mpi_calls_list(void) {

    return &(mpi_calls_opts[0]);
}

int get_call_index(char* name) {
    int index = -1;
    int i;

    for (i=0; i< N_MPI_CALLS; i++) {
        if (strcmp(name, mpi_calls_opts[i]) == 0) {
            index = i;
            break;
        }
    }

    return index;
}



char* get_call_from_index(int index) {
    if (index < 0 || index >= N_MPI_CALLS) {
        return "";
    }
    return strdup(mpi_calls_opts[index]);
}


inline void execute_Empty(collective_params_t* params) {
}



void initialize_data_default(const basic_collective_params_t info, const long count,
        collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
    memset(params->sbuf, 0, params->scount * params->datatype_extent);
    memset(params->rbuf, 0, params->rcount * params->datatype_extent);

}


void cleanup_data_default(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}


void initialize_common_data(const basic_collective_params_t info,
        collective_params_t* params) {

    MPI_Aint lb;

    MPI_Type_get_extent(info.datatype, &lb, &(params->datatype_extent));
    params->datatype = info.datatype;

    params->op = info.op;

    MPI_Comm_rank(MPI_COMM_WORLD, &params->rank);
    params->nprocs = info.nprocs;
    assert(params->nprocs > 0);

    params->root = info.root;

    params->pingpong_ranks[0] = info.pingpong_ranks[0];
    params->pingpong_ranks[1] = info.pingpong_ranks[1];

    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
    params->counts_array = NULL;
    params->displ_array = NULL;
}


void init_collective_basic_info(reprompib_common_options_t opts, int procs, basic_collective_params_t* coll_basic_info) {
    // initialize common collective calls information
    coll_basic_info->datatype = opts.datatype;
    coll_basic_info->nprocs = procs;
    coll_basic_info->op = opts.operation;
    coll_basic_info->root = 0;

    coll_basic_info->pingpong_ranks[0] = opts.pingpong_ranks[0];
    coll_basic_info->pingpong_ranks[1] = opts.pingpong_ranks[1];

    if (opts.root_proc >= 0 && opts.root_proc < procs) {
        coll_basic_info->root = opts.root_proc;
    }

}





