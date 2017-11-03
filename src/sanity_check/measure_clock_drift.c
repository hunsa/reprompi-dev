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
#include <time.h>
#include <math.h>
#include "mpi.h"

#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "parse_test_options.h"

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_sort.h>

static const int RTT_WARMUP_ROUNDS = 5;
static const int OUTPUT_ROOT_PROC = 0;
static const int NREP_MEAS = 10;

void estimate_all_rtts(int master_rank, int other_rank, const int n_pingpongs,
        double *rtt) {
    int my_rank, np;
    MPI_Status stat;
    int i;
    double tmp;
    double *rtts = NULL;
    double mean;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == master_rank) {
        double tstart, tremote;

        /* warm up */
        for (i = 0; i < RTT_WARMUP_ROUNDS; i++) {
            tmp = get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD);
            MPI_Recv(&tmp, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD, &stat);
        }

        rtts = (double*) malloc(n_pingpongs * sizeof(double));

        for (i = 0; i < n_pingpongs; i++) {
            tstart = get_time();
            MPI_Send(&tstart, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD);
            MPI_Recv(&tremote, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD,
                    &stat);
            rtts[i] = get_time() - tstart;
        }

    } else if (my_rank == other_rank) {
        double tlocal = 0, troot;

        /* warm up */
        for (i = 0; i < RTT_WARMUP_ROUNDS; i++) {
            MPI_Recv(&tmp, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD,
                    &stat);
            tmp = get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD);
        }

        for (i = 0; i < n_pingpongs; i++) {
            MPI_Recv(&troot, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD,
                    &stat);
            tlocal = get_time();
            MPI_Send(&tlocal, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD);
        }
    }

    if (my_rank == master_rank) {
        double upperq;
        double cutoff_val;
        double *rtts2;
        int n_datapoints;

        gsl_sort(rtts, 1, n_pingpongs);

        upperq = gsl_stats_quantile_from_sorted_data(rtts, 1, n_pingpongs,
                0.75);
        cutoff_val = 1.5 * upperq;

        rtts2 = (double*) calloc(n_pingpongs, sizeof(double));
        n_datapoints = 0;
        for (i = 0; i < n_pingpongs; i++) {
            if (rtts[i] <= cutoff_val) {
                rtts2[i] = rtts[i];
                n_datapoints = i + 1;
            } else {
                break;
            }
        }

        mean = gsl_stats_mean(rtts2, 1, n_datapoints);

        free(rtts);
        free(rtts2);

        MPI_Send(&mean, 1, MPI_DOUBLE, other_rank, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&mean, 1, MPI_DOUBLE, master_rank, 0, MPI_COMM_WORLD, &stat);
    }

    *rtt = mean;
}

void print_initial_settings(int argc, char* argv[], reprompib_st_opts_t opts,
        print_sync_info_t print_sync_info) {
    int my_rank, np;
    FILE * f;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;

        fprintf(f, "#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            fprintf(f, " %s", argv[i]);
        }
        fprintf(f, "\n");
        fprintf(f, "#@nrep=%ld\n", opts.n_rep);
        fprintf(f, "#@steps=%d\n", opts.steps);
        fprintf(f, "#@timerres=%14.9f\n", MPI_Wtick());

        print_time_parameters(f);
        print_sync_info(f);
    }

}

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    int i;
    reprompib_st_opts_t opts;
    int master_rank;
    MPI_Status stat;
    double* rtts_s;
    FILE* f;
    reprompib_sync_module_t sync_module;
    reprompib_sync_params_t sync_params;


    int n_pingpongs = 1000;
    double global_time, local_time;

    double *all_local_times = NULL;
    double *all_global_times = NULL;
    double time_msg[2];

    int step;
    int n_wait_steps = 0;
    double wait_time_s = 1;
    struct timespec sleep_time;
    double runtime_s;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    reprompib_register_sync_modules();

    parse_test_options(&opts, argc, argv);

    reprompib_init_sync_module(argc, argv, &sync_module);
    init_timer();

    n_wait_steps = opts.steps + 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // compute RTTs
    n_pingpongs = 1000;
    p = 0;
    rtts_s = (double*) calloc(nprocs, sizeof(double));
    if (my_rank == master_rank) {
        for (p = 0; p < nprocs; p++) {
            if (p != master_rank) {
                estimate_all_rtts(master_rank, p, n_pingpongs, &rtts_s[p]);
            }
            //printf("rtt to %d: %14.9f\n", p, rtts_s[p]);
        }
    } else {
        estimate_all_rtts(master_rank, my_rank, n_pingpongs, &rtts_s[p]);
    }

    if (my_rank == master_rank) {
        all_global_times = (double*) calloc(nprocs * opts.n_rep * n_wait_steps,
                sizeof(double));
        all_local_times = (double*) calloc(nprocs * opts.n_rep * n_wait_steps,
                sizeof(double));
    }


    print_initial_settings(argc, argv, opts, sync_module.print_sync_info);

    runtime_s = get_time();
    sync_params.nrep = NREP_MEAS;
    sync_module.init_sync(&sync_params);

    sync_module.sync_clocks();
    runtime_s = get_time() - runtime_s;

    if (my_rank == master_rank) {
        printf ("#@sync_duration=%14.9f\n", runtime_s);
    }
    for (step = 0; step < n_wait_steps; step++) {
        if (my_rank == master_rank) {

            for (p = 0; p < nprocs; p++) {
                for (i = 0; i < opts.n_rep; i++) {
                    if (p != master_rank) {
                        MPI_Send(&time_msg[0], 2, MPI_DOUBLE, p, 0,
                                MPI_COMM_WORLD);
                        MPI_Recv(&time_msg[0], 2, MPI_DOUBLE, p, 0,
                                MPI_COMM_WORLD, &stat);

                        // cannot use the local time on the root as a reference time, it needs to be
                        // normalized according to the synchronization method
                        // (for JK, SKaMPI, on the root process global_time  = local_time)
                        all_local_times[step * nprocs * opts.n_rep + p * opts.n_rep + i] =
                            sync_module.get_global_time(get_time()) - rtts_s[p] / 2;
                        all_global_times[step * nprocs * opts.n_rep
                                         + p * opts.n_rep + i] = time_msg[1];

                    }
                }
            }

            // wait 1 second
            sleep_time.tv_sec = wait_time_s;
            sleep_time.tv_nsec = 0;

            nanosleep(&sleep_time, &sleep_time);

        } else {
            for (i = 0; i < opts.n_rep; i++) {
                MPI_Recv(&time_msg[0], 2, MPI_DOUBLE, master_rank, 0,
                        MPI_COMM_WORLD, &stat);

                local_time = get_time();
                global_time = sync_module.get_global_time(local_time);
                time_msg[0] = local_time;
                time_msg[1] = global_time;

                MPI_Send(&time_msg[0], 2, MPI_DOUBLE, master_rank, 0,
                        MPI_COMM_WORLD);

            }

        }

    }
    sync_module.finalize_sync();

    f = stdout;
    if (my_rank == master_rank) {
        fprintf(f,"%14s %3s %4s %14s %14s %14s\n", "wait_time_s", "p", "rep", "gtime", "reftime", "diff");
    }

    for (step = 0; step < n_wait_steps; step++) {
        if (my_rank == master_rank) {
            for (p = 0; p < nprocs; p++) {
                if (p != master_rank) {
                    for (i = 0; i < opts.n_rep; i++) {
                        global_time = all_global_times[step * nprocs
                                                       * opts.n_rep + p * opts.n_rep + i];
                        local_time = all_local_times[step * nprocs * opts.n_rep
                                                     + p * opts.n_rep + i];
                        //global_time_comp = local_time / linear_models[p].slope - linear_models[p].intercept  / linear_models[p].slope;
                        fprintf(f, "%14.9f %3d %4d %14.9f %14.9f %14.9f\n",
                                step * wait_time_s, p, i, global_time,
                                local_time, global_time - local_time);
                    }
                }
            }
        }
    }

    if (my_rank == master_rank) {
        free(all_local_times);
        free(all_global_times);
    }

    sync_module.cleanup_module();
    reprompib_deregister_sync_modules();
    MPI_Finalize();
    free(rtts_s);
    return 0;
}
