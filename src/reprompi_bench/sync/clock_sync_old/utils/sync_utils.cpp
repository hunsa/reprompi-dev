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

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <reprompi_bench/sync/clock_sync/clocks/Clock.h>

#include "sync_utils.h"

void compute_rtt(int master_rank, int other_rank, MPI_Comm comm, int nwarmups, int n_pingpongs, Clock& c,  double *rtt) {
    int my_rank, np;
    MPI_Status stat;
    int i;
    double tmp;
    double *rtts = NULL;
    double mean;

    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &np);

    if (my_rank == master_rank) {
        double tstart, tremote;

        /* warm up */
        for (i = 0; i < nwarmups; i++) {
            tmp = c.get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, other_rank, 0, comm);
            MPI_Recv(&tmp, 1, MPI_DOUBLE, other_rank, 0, comm, &stat);
        }

        rtts  = new double[n_pingpongs];

        for (i = 0; i < n_pingpongs; i++) {
            tstart = c.get_time();
            MPI_Send(&tstart, 1, MPI_DOUBLE, other_rank, 0, comm);
            MPI_Recv(&tremote, 1, MPI_DOUBLE, other_rank, 0, comm, &stat);
            rtts[i] = c.get_time() - tstart;
        }

    } else if( my_rank == other_rank ) {
        double tlocal = 0, troot;

        /* warm up */
        for (i = 0; i < nwarmups; i++) {
            MPI_Recv(&tmp, 1, MPI_DOUBLE, master_rank, 0, comm, &stat);
            tmp = c.get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, master_rank, 0, comm);
        }

        for (i = 0; i < n_pingpongs; i++) {
            MPI_Recv(&troot, 1, MPI_DOUBLE, master_rank, 0, comm, &stat);
            tlocal = c.get_time();
            MPI_Send(&tlocal, 1, MPI_DOUBLE, master_rank, 0, comm);
        }
    }
    if (my_rank == master_rank) {
        double upperq;
        double cutoff_val;
        double *rtts2;
        int n_datapoints;

        gsl_sort(rtts, 1, n_pingpongs);

        upperq = gsl_stats_quantile_from_sorted_data (rtts, 1, n_pingpongs, 0.75);
        cutoff_val = 1.5 * upperq;

        //    printf("cutoff=%.20f\n", cutoff_val);
        rtts2 = new double[n_pingpongs];
        n_datapoints = 0;
        for(i=0; i<n_pingpongs; i++) {
            if( rtts[i] <= cutoff_val ) {
                rtts2[i] = rtts[i];
                n_datapoints = i+1;
            } else {
                break;
            }
        }
        mean = gsl_stats_mean(rtts2, 1, n_datapoints);

        delete[] rtts;
        delete[] rtts2;

        MPI_Send(&mean, 1, MPI_DOUBLE, other_rank, 0, comm);
    } else {
        MPI_Recv(&mean, 1, MPI_DOUBLE, master_rank, 0, comm, &stat);
    }
    *rtt = mean;
}



