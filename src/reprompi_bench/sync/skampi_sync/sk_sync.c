/*  ReproMPI Benchmark
 *
 * Copyright 2003-2008 Werner Augustin, Lars Diesselberg - SKaMPI   MPI-Benchmark
   Lehrstuhl Informatik fuer Naturwissenschaftler und Ingenieure
   Fakultaet fuer Informatik
   University of Karlsruhe
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

#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/sync/sync_info.h"
#include "sk_parse_options.h"
#include "reprompi_bench/sync/synchronization.h"

static double *tds; /* tds[i] is the time difference between the
 current node and global node i */

static int *invalid; /* invalid[max_count_repetitions] */
static int repetition_counter;

static double *ping_pong_min_time; /* ping_pong_min_time[i] is the minimum time of one ping_pong
 between the current node and node i, negative value means
 time not yet determined;
 needed to avoid measuring again all the 100 RTTs when re-synchronizing
 (in this case only a few ping-pongs are performed if the RTT stays
 within 110% for the ping_pong_min_time)
 */

static double start_batch, start_sync, stop_sync;
static int sync_index = 0; /* current window index within the current measurement batch */

typedef struct {
    long n_rep; /* --repetitions */
    double window_size_sec; /* --window-size */

    double wait_time_sec; /* --wait-time */
} reprompi_sk_options_t;

// options specified from the command line
static reprompi_sk_options_t parameters;

enum {
    Number_ping_pongs = 100,
    Minimum_ping_pongs = 8
};


static void* skampi_malloc(int size);
static void mpi_abort(int errorcode);
static void error_with_abort(int errorcode, char *fmt, ...);


static void print_global_time_differences() {
    int i, p, name_len, pid;
    char my_name[MPI_MAX_PROCESSOR_NAME];

    double *all_tds = NULL;
    char *names = NULL;
    int *pids = NULL;

    int my_rank, np;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == root_proc) {
        all_tds = (double*) skampi_malloc(np * np * sizeof(double));
        names = (char*) skampi_malloc(np * MPI_MAX_PROCESSOR_NAME);
        pids = (int*) skampi_malloc(np * sizeof(int));
    }

    MPI_Get_processor_name(my_name, &name_len);
    my_name[name_len] = '\0';
    pid = getpid();

    MPI_Gather(tds, np, MPI_DOUBLE, all_tds, np,
            MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(my_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, names,
            MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Gather(&pid, 1, MPI_INT, pids, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank == root_proc) {
        for (i = 0; i < np; i++)
            printf("#name[%d] = \"%s\" pid=%d\n", i,
                    &(names[i * MPI_MAX_PROCESSOR_NAME]), pids[i]);

        for (p = 0; p < np; p++) {
            printf("#tds[%d -> ..] = [%9.6f", p,
                    all_tds[p * np + 0]);
            for (i = 1; i < np; i++)
                printf(", %9.6f", all_tds[p * np + i]);
            printf("]\n");
        }

        free(all_tds);
        free(names);
        free(pids);
    }
}

/*----------------------------------------------------------------------------*/

static void ping_pong(int p1, int p2, int my_rank, int np) {
    int i, other_global_id;
    double s_now, s_last, t_last, t_now;
    double td_min, td_max;
    double invalid_time = -1.0;
    MPI_Status status;
    int pp_tag = 43;

    ping_pong_min_time = (double*) skampi_malloc(
            np * sizeof(double));
    for (i = 0; i < np; i++)
        ping_pong_min_time[i] = -1.0;

    /* I had to unroll the main loop because I didn't find a portable way
     to define the initial td_min and td_max with INFINITY and NINFINITY */
    if (my_rank == p1) {
        other_global_id = p2;

        s_last = get_time();
        MPI_Send(&s_last, 1, MPI_DOUBLE, p2, pp_tag, MPI_COMM_WORLD);
        MPI_Recv(&t_last, 1, MPI_DOUBLE, p2, pp_tag, MPI_COMM_WORLD, &status);
        s_now = get_time();
        MPI_Send(&s_now, 1, MPI_DOUBLE, p2, pp_tag, MPI_COMM_WORLD);

        td_min = t_last - s_now;
        td_max = t_last - s_last;

    } else {
        other_global_id = p1;

        MPI_Recv(&s_last, 1, MPI_DOUBLE, p1, pp_tag, MPI_COMM_WORLD, &status);
        t_last = get_time();
        MPI_Send(&t_last, 1, MPI_DOUBLE, p1, pp_tag, MPI_COMM_WORLD);
        MPI_Recv(&s_now, 1, MPI_DOUBLE, p1, pp_tag, MPI_COMM_WORLD, &status);
        t_now = get_time();

        td_min = s_last - t_last;
        td_min = repro_max(td_min, s_now - t_now);

        td_max = s_now - t_last;
    }

    if (my_rank == p1) {
        i = 1;
        while (1) {

            MPI_Recv(&t_last, 1, MPI_DOUBLE, p2, pp_tag, MPI_COMM_WORLD,
                    &status);
            if (t_last < 0.0) {
                break;
            }

            s_last = s_now;
            s_now = get_time();

            td_min = repro_max(td_min, t_last - s_now);
            td_max = repro_min(td_max, t_last - s_last);

            if (ping_pong_min_time[other_global_id] >= 0.0
                    && i >= Minimum_ping_pongs
                    && s_now - s_last
                    < ping_pong_min_time[other_global_id] * 1.10) {
                MPI_Send(&invalid_time, 1, MPI_DOUBLE, p2, pp_tag,
                        MPI_COMM_WORLD);
                break;
            }
            i++;
            if (i == Number_ping_pongs) {
                MPI_Send(&invalid_time, 1, MPI_DOUBLE, p2, pp_tag,
                        MPI_COMM_WORLD);
                break;
            }
            MPI_Send(&s_now, 1, MPI_DOUBLE, p2, pp_tag, MPI_COMM_WORLD);

        }
    } else {
        i = 1;
        while (1) {
            MPI_Send(&t_now, 1, MPI_DOUBLE, p1, pp_tag, MPI_COMM_WORLD);
            MPI_Recv(&s_last, 1, MPI_DOUBLE, p1, pp_tag, MPI_COMM_WORLD,
                    &status);
            t_last = t_now;
            t_now = get_time();

            if (s_last < 0.0) {
                break;
            }

            td_min = repro_max(td_min, s_last - t_now);
            td_max = repro_min(td_max, s_last - t_last);

            if (ping_pong_min_time[other_global_id] >= 0.0
                    && i >= Minimum_ping_pongs
                    && t_now - t_last
                    < ping_pong_min_time[other_global_id] * 1.10) {
                MPI_Send(&invalid_time, 1, MPI_DOUBLE, p1, pp_tag,
                        MPI_COMM_WORLD);
                break;
            }
            i++;
        }
    }

    if (ping_pong_min_time[other_global_id] < 0.0) {
        ping_pong_min_time[other_global_id] = td_max - td_min;
    }
    else {
        ping_pong_min_time[other_global_id] = repro_min(
                ping_pong_min_time[other_global_id], td_max - td_min);
    }

    tds[other_global_id] = (td_min + td_max) / 2.0;

    free(ping_pong_min_time);

}

static void determine_time_differences(int my_rank, int np) {
    int i;
    double *tmp_tds;

    //  measure ping-pong time between processes 0 and i
    for (i = 1; i < np; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (my_rank == 0 || my_rank == i)
            ping_pong(0, i, my_rank, np);
    }

    // send root time differences to all the other processes
    tmp_tds = (double*) skampi_malloc(np * sizeof(double));
    if (my_rank == 0) {
        for (i = 1; i < np; i++)
            tmp_tds[i] = tds[i];
    }

    assert(np - 1 >= 0);
    MPI_Bcast(&(tmp_tds[1]), np - 1, MPI_DOUBLE, 0,
            MPI_COMM_WORLD);

    // update local time differences
    if (my_rank != 0) {
        for (i = 1; i < np; i++) {
            tds[i] = tmp_tds[i] + tds[0];
        }
    }
    free(tmp_tds);
    MPI_Barrier(MPI_COMM_WORLD);

}


/*---------------------------------------------------------------------------*/

static int wait_till(double time_stamp, double *last_time_stamp) {
    if ((*last_time_stamp = get_time()) > time_stamp) {
        return 0;
    }
    else {
        while ((*last_time_stamp = get_time()) < time_stamp) {
            //
        }
        return 1;
    }
}

static inline double should_wait_till(int counter, double interval, double offset) {
    return (counter + 1) * interval + start_batch + offset;
}

static void sk_sync_clocks(void) {
    int my_rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    determine_time_differences(my_rank, np);
}

static void sk_init_synchronization(const reprompib_sync_params_t* init_params) {
    int np, i;
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    parameters.n_rep = init_params->nrep;

    tds = (double*) skampi_malloc(np * sizeof(double));
    for (i = 0; i < np; i++)
        tds[i] = 0.0;

    invalid = (int*) skampi_malloc(parameters.n_rep * sizeof(int));
    for (i = 0; i < parameters.n_rep; i++) {
        invalid[i] = 0;
    }

    repetition_counter = 0;
}

static void sk_finalize_synchronization(void) {
    free(tds);
    free(invalid);
}

static void sk_init_sync_round(void) { // initialize the first window
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank == 0) {
      start_batch = get_time() + parameters.wait_time_sec;
  }

  MPI_Bcast(&start_batch, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

static void sk_start_synchronization(void) {
    if (!wait_till(
            should_wait_till(sync_index, parameters.window_size_sec, -tds[0]),
            &start_sync)) {
        invalid[repetition_counter] |= FLAG_START_TIME_HAS_PASSED;
    }
    else {
        invalid[repetition_counter] = 0;
    }
}

static void sk_stop_synchronization(void) {
    stop_sync = get_time();

    if (stop_sync - start_sync > parameters.window_size_sec) {
        invalid[repetition_counter] |= FLAG_SYNC_WIN_EXPIRED;
    }

    repetition_counter++;
    sync_index++;
}

static void print_sync_results(void) {
    int* sync_res = NULL;
    int i, j;
    int my_rank, np;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == root_proc) {
        sync_res = (int*) malloc(
                parameters.n_rep * np * sizeof(int));
    }

    MPI_Gather(invalid, parameters.n_rep, MPI_INT, sync_res, parameters.n_rep,
            MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank == root_proc) {
        for (i = 0; i < np; i++) {
            for (j = 0; j < parameters.n_rep; j++) {
                printf("#%d %d %d\n", i, j, sync_res[i * parameters.n_rep + j]);
            }
        }
        free(sync_res);
    }
}

static int* sk_get_local_sync_errorcodes(void) {
    return invalid;
}


static void sk_print_sync_parameters(FILE* f) {
    fprintf(f, "#@sync=SKaMPI\n");
    fprintf(f, "#@window_s=%.10f\n", parameters.window_size_sec);
    fprintf(f, "#@wait_time_s=%.10f\n", parameters.wait_time_sec);
}

static inline double sk_get_timediff_to_root(void) {
    return tds[0];
}

static inline double sk_get_normalized_time(double local_time) {
    return local_time + tds[0];
}


static void* skampi_malloc(int size) {
    void *baseptr = NULL;

    if (size < 0) {
        error_with_abort(MPI_ERR_ARG,
                "\nskampi_malloc: Invalid value for size, must be non-negative but is %d"
                "\nskampi_malloc: Caller in file %s at line %u\n",
                (int) size, __FILE__, __LINE__);
    }
    baseptr = malloc(size);
    if (baseptr == NULL) {
        error_with_abort(MPI_ERR_ARG, "\nskampi_malloc: malloc(size=%d) failed."
                "\nskampi_malloc: %s (%d)"
                "\nskampi_malloc: Caller in file %s at line %u\n",
                (size_t) size, strerror(errno), errno, __FILE__, __LINE__);
    }

    return baseptr;
}

static void error_with_abort(int errorcode, char *fmt, ...) {
    va_list args;

    /* we try to send a message to stderr whether we officialy can do I/O
     (i_can_do_io) or not */
    /*  if( i_can_do_io ) {*/
    va_start(args, fmt);
    fprintf(stderr, "error code %d: ", errorcode);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
    /* } */
    mpi_abort(errorcode);
}

static void mpi_abort(int errorcode) {
    /* we try to send a message to stderr whether we officialy can do I/O
     (i_can_do_io) or not */
    /*  if( i_can_do_io ) {*/
    fprintf(stderr, "\n");
    fflush(stderr);
    /* } */
    MPI_Abort(MPI_COMM_WORLD, errorcode);
}


void sk_init_module(int argc, char** argv) {
  reprompib_sync_options_t sync_opts;
  sk_parse_options(argc, argv, &sync_opts);

  parameters.wait_time_sec = sync_opts.wait_time_sec;
  parameters.window_size_sec = sync_opts.window_size_sec;
}


void sk_cleanup_module(void) {

}


void register_skampi_module(reprompib_sync_module_t *sync_mod) {
  sync_mod->name = "SKaMPI";
  sync_mod->clocksync = REPROMPI_CLOCKSYNC_SKAMPI;
  sync_mod->sync_type = REPROMPI_SYNCTYPE_WIN;

  sync_mod->init_module = sk_init_module;
  sync_mod->cleanup_module = sk_cleanup_module;

  sync_mod->init_sync = sk_init_synchronization;
  sync_mod->finalize_sync = sk_finalize_synchronization;

  sync_mod->start_sync = sk_start_synchronization;
  sync_mod->stop_sync = sk_stop_synchronization;

  sync_mod->sync_clocks = sk_sync_clocks;
  sync_mod->init_sync_round = sk_init_sync_round;

  sync_mod->get_global_time = sk_get_normalized_time;
  sync_mod->get_errorcodes = sk_get_local_sync_errorcodes;
  sync_mod->print_sync_info = sk_print_sync_parameters;
}






