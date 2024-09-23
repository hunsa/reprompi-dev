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
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include "mpi.h"

#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reproMPIbenchmark.h"
#include "results_output.h"
#include "reprompi_bench/sync/process_sync/process_synchronization.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_results_header(const reprompib_lib_output_info_t* output_info_p,
    const reprompib_job_t* job_p,
    const reprompib_sync_module_t* clock_sync_module,
    const reprompib_proc_sync_module_t* proc_sync_module)
{
    FILE* f = stdout;
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;

        for (i=0; i<job_p->n_user_svars; i++) {
            fprintf(f, "%30s ", job_p->user_svar_names[i]);
        }
        for (i=0; i<job_p->n_user_ivars; i++) {
            fprintf(f, "%10s ", job_p->user_ivar_names[i]);
        }

        fprintf(f, "%20s %7s", "measure_type", "proc");

        if (output_info_p->verbose == 1 && output_info_p->print_summary_methods == 0) {
          if (proc_sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
            fprintf(f, " %12s", "errorcode");
            fprintf(f," %8s %16s %16s %16s %16s\n", "nrep", "loc_tstart_sec", "loc_tend_sec", "gl_tstart_sec", "gl_tend_sec");
          }
          else {
            fprintf(f," %8s %16s %16s\n",  "nrep", "loc_tstart_sec", "loc_tend_sec");
          }
        } else {

            // print summary
            if (output_info_p->print_summary_methods >0) {
              fprintf(f, " %8s %8s ", "total_nrep", "valid_nrep");

              for (i=0; i<reprompib_get_number_summary_methods(); i++) {
                summary_method_info_t* s = reprompib_get_summary_method(i);
                if (output_info_p->print_summary_methods & s->mask) {
                  fprintf(f, "%10s_sec ", s->name);
                }
              }
              fprintf(f, "\n");
            }
            else {
              if (proc_sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
                fprintf(f, " %12s", "errorcode");
              }
              fprintf(f, " %8s %16s\n", "nrep", "runtime_sec");
            }
        }

    }
}


void compute_runtimes_local_clocks_with_reduction(
        const double* tstart_sec, const double* tend_sec,
        long current_start_index, long current_nreps,
        double* maxRuntimes_sec, char* op) {

    double* local_runtimes = NULL;
    int i, index;
    int my_rank, np;
    MPI_Op operation = MPI_MAX;

    if (strcmp("min", op) == 0) {
        operation = MPI_MIN;
    }
    if (strcmp("mean", op) == 0) {
        operation = MPI_SUM;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // compute local runtimes for the [current_start_index, current_start_index + current_nreps) interval
    local_runtimes = (double*) malloc(current_nreps * sizeof(double));
    for (i = 0; i < current_nreps; i++) {
        index = i + current_start_index;
        local_runtimes[i] = tend_sec[index] - tstart_sec[index];
    }

    // reduce local measurement results on the root
    MPI_Reduce(local_runtimes, maxRuntimes_sec, current_nreps,
            MPI_DOUBLE, operation, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (strcmp("mean", op) == 0) {      // reduce with sum and then compute mean
            for (i = 0; i < current_nreps; i++) {
                maxRuntimes_sec[i] = maxRuntimes_sec[i]/np;
            }
        }
    }
    free(local_runtimes);
}



void print_runtimes(FILE* f, const reprompib_job_t* job_p,
    const reprompib_sync_module_t* clock_sync_module,
    const reprompib_proc_sync_module_t* sync_module) {

    double* maxRuntimes_sec;
    int i;
    int my_rank;
    int* sync_errorcodes;
    long current_start_index;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    maxRuntimes_sec = NULL;
    sync_errorcodes = NULL;
    if (my_rank == OUTPUT_ROOT_PROC) {
        maxRuntimes_sec = (double*) malloc(job_p->n_rep * sizeof(double));

        if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
          sync_errorcodes = (int*) malloc(job_p->n_rep * sizeof(int));
          for (i = 0; i < job_p->n_rep; i++) {
            sync_errorcodes[i] = 0;
          }
        }
    }

    current_start_index = 0;

    if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
      collect_errorcodes(current_start_index, job_p->n_rep, OUTPUT_ROOT_PROC, sync_module->get_errorcodes, sync_errorcodes);
      compute_runtimes_global_clocks(job_p->tstart_sec, job_p->tend_sec, current_start_index, job_p->n_rep, OUTPUT_ROOT_PROC,
          clock_sync_module->get_global_time, maxRuntimes_sec);
    }
    else {
      compute_runtimes_local_clocks_with_reduction(job_p->tstart_sec, job_p->tend_sec, current_start_index, job_p->n_rep,
          maxRuntimes_sec, job_p->op);
    }

    if (my_rank == OUTPUT_ROOT_PROC) {
        for (i = 0; i < job_p->n_rep; i++) {
            int j;

            for (j=0; j<job_p->n_user_svars; j++) {
                fprintf(f, "%30s ", job_p->user_svars[j]);
            }
            for (j=0; j<job_p->n_user_ivars; j++) {
                fprintf(f, "%10d ", job_p->user_ivars[j]);
            }

            // measurements with window-based synchronization
            if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
              fprintf(f, "%20s %7s %12d %8d %16.10f\n", job_p->timername, job_p->timertype,
                    sync_errorcodes[i],i,
                    maxRuntimes_sec[i]);
            }
            else {
              // measurements with Barrier-based synchronization
              fprintf(f, "%20s %7s %8d %16.10f\n", job_p->timername,  job_p->timertype,
                    i, maxRuntimes_sec[i]);
            }
        }

        if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
          free(sync_errorcodes);
        }
        free(maxRuntimes_sec);
    }
}



void print_runtimes_allprocs(FILE* f,
    const reprompib_job_t* job_p,
    const reprompib_sync_module_t* clock_sync_module,
    const reprompib_proc_sync_module_t* sync_module,
    const double* global_start_sec, const double* global_end_sec,
    int* errorcodes) {

    double* maxRuntimes_sec;
    int i;
    int my_rank;
    int np;
    int proc_id;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    maxRuntimes_sec = NULL;
    if (my_rank == OUTPUT_ROOT_PROC) {
        maxRuntimes_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));

        for (proc_id = 0; proc_id < np; proc_id++) {
            for (i = 0; i < job_p->n_rep; i++) {
                maxRuntimes_sec[proc_id * job_p->n_rep + i] = global_end_sec[proc_id * job_p->n_rep + i] -
                        global_start_sec[proc_id * job_p->n_rep+ i];
            }
        }

        for (proc_id = 0; proc_id < np; proc_id++) {
            for (i = 0; i < job_p->n_rep; i++) {
                int j;

                for (j=0; j<job_p->n_user_svars; j++) {
                    fprintf(f, "%30s ", job_p->user_svars[j]);
                }
                for (j=0; j<job_p->n_user_ivars; j++) {
                    fprintf(f, "%10d ", job_p->user_ivars[j]);
                }

                if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
                  // measurements with window-based synchronization
                  fprintf(f, "%20s %4d %12d %8d %16.10f\n", job_p->timername,
                        proc_id, errorcodes[proc_id * job_p->n_rep + i], i,
                        maxRuntimes_sec[proc_id * job_p->n_rep + i]);
                }
                else {
                  // measurements with Barrier-based synchronization
                  fprintf(f, "%20s %4d %8d %16.10f\n", job_p->timername,
                        proc_id,i, maxRuntimes_sec[proc_id * job_p->n_rep + i]);
                }
            }
        }

        free(maxRuntimes_sec);
    }
}


void print_measurement_results(FILE* f,
    const reprompib_lib_output_info_t* output_info_p,
    const reprompib_job_t* job_p,
    const reprompib_sync_module_t* clock_sync_module,
    const reprompib_proc_sync_module_t* proc_sync_module) {

    int i, proc_id;
    double* local_start_sec = NULL;
    double* local_end_sec = NULL;
    double* global_start_sec = NULL;
    double* global_end_sec = NULL;
    double* tmp_local_start_sec = NULL;
    double* tmp_local_end_sec = NULL;
    int my_rank, np;
    int* errorcodes = NULL;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    //printf("print verbose: %d\n", output_info_p->verbose);

    if (output_info_p->verbose == 0 && strcmp(job_p->timertype, "all") != 0) {
        print_runtimes(f, job_p, clock_sync_module, proc_sync_module);

    } else {

      if (proc_sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
        int* local_errorcodes = proc_sync_module->get_errorcodes();

        if (my_rank == OUTPUT_ROOT_PROC)
        {
            errorcodes = (int*)malloc(job_p->n_rep * np * sizeof(int));
            for (i = 0; i < job_p->n_rep * np; i++) {
                errorcodes[i] = 0;
            }
        }

            MPI_Gather(local_errorcodes, job_p->n_rep, MPI_INT,
                    errorcodes, job_p->n_rep, MPI_INT, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);
      }

      if (my_rank == OUTPUT_ROOT_PROC) {
            local_start_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            local_end_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            global_start_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            global_end_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
        }

        // gather measurement results
        MPI_Gather(job_p->tstart_sec, job_p->n_rep, MPI_DOUBLE,
            local_start_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        MPI_Gather(job_p->tend_sec, job_p->n_rep, MPI_DOUBLE,
            local_end_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        tmp_local_start_sec = (double*) malloc(
            job_p->n_rep * np * sizeof(double));
        tmp_local_end_sec = (double*) malloc(
            job_p->n_rep * np * sizeof(double));
        for (i = 0; i < job_p->n_rep; i++) {
            tmp_local_start_sec[i] = clock_sync_module->get_global_time(job_p->tstart_sec[i]);
            tmp_local_end_sec[i] = clock_sync_module->get_global_time(job_p->tend_sec[i]);
        }
        MPI_Gather(tmp_local_start_sec, job_p->n_rep, MPI_DOUBLE,
            global_start_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        MPI_Gather(tmp_local_end_sec, job_p->n_rep, MPI_DOUBLE,
            global_end_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        free(tmp_local_start_sec);
        free(tmp_local_end_sec);

        if (output_info_p->verbose == 0  && strcmp(job_p->timertype, "all") == 0) {
          print_runtimes_allprocs(f, job_p, clock_sync_module, proc_sync_module, global_start_sec, global_end_sec,
                                  errorcodes);
        } else {      // verbose == 1

            if (my_rank == OUTPUT_ROOT_PROC) {
                for (proc_id = 0; proc_id < np; proc_id++) {
                    for (i = 0; i < job_p->n_rep; i++) {
                        int j;
                        for (j=0; j< job_p->n_user_svars; j++) {
                            fprintf(f, "%30s ", job_p->user_svars[j]);
                        }
                        for (j=0; j<job_p->n_user_ivars; j++) {
                            fprintf(f, "%10d ", job_p->user_ivars[j]);
                        }

                        if (proc_sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
                          fprintf(f, "%20s %4d %12d %8d %16.10f %16.10f %16.10f %16.10f\n",
                            job_p->timername, proc_id,
                                errorcodes[proc_id * job_p->n_rep + i], i,
                                local_start_sec[proc_id * job_p->n_rep + i],
                                local_end_sec[proc_id * job_p->n_rep + i],
                                global_start_sec[proc_id * job_p->n_rep + i],
                                global_end_sec[proc_id * job_p->n_rep + i]);
                        }
                        else {
                          fprintf(f, "%20s %4d %8d %16.10f %16.10f\n", job_p->timername,
                                proc_id, i,
                                local_start_sec[proc_id * job_p->n_rep + i],
                                local_end_sec[proc_id * job_p->n_rep + i]);
                        }
                    }
                }
            }
        }

        if (my_rank == OUTPUT_ROOT_PROC) {
            free(local_start_sec);
            free(local_end_sec);
            free(global_start_sec);
            free(global_end_sec);
            if (proc_sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
              free(errorcodes);
            }
        }

    }
}



void print_summary(FILE* f,
        const reprompib_lib_output_info_t* output_info_p,
        const reprompib_job_t* job_p,
        const reprompib_sync_module_t* clock_sync_module,
        const reprompib_proc_sync_module_t* sync_module) {

    double* maxRuntimes_sec;
    int i, j;
    int my_rank, np;
    int* sync_errorcodes;
    long current_start_index;
    int n_results = 0, proc;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    maxRuntimes_sec = NULL;
    sync_errorcodes = NULL;
    if (my_rank == OUTPUT_ROOT_PROC) {
        maxRuntimes_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));

        if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
          sync_errorcodes = (int*) malloc(job_p->n_rep * np * sizeof(int));
          for (i = 0; i < job_p->n_rep * np; i++) {
            sync_errorcodes[i] = 0;
          }
        }
    }

    current_start_index = 0;

    if (strcmp(job_p->timertype, "all") !=0) { // one runtime for each nrep id (reduced over processes)

      if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
        collect_errorcodes(current_start_index, job_p->n_rep, OUTPUT_ROOT_PROC, sync_module->get_errorcodes, sync_errorcodes);
        compute_runtimes_global_clocks(job_p->tstart_sec, job_p->tend_sec,
                current_start_index, job_p->n_rep, OUTPUT_ROOT_PROC,
                clock_sync_module->get_global_time, maxRuntimes_sec);
      }
      else {
        compute_runtimes_local_clocks_with_reduction(job_p->tstart_sec, job_p->tend_sec, current_start_index, job_p->n_rep,
                maxRuntimes_sec, job_p->op);
      }
      n_results = 1;

    }
    else {  // one runtime for each process and each nrep
        double* local_start_sec = NULL;
        double* local_end_sec = NULL;
        double* global_start_sec = NULL;
        double* global_end_sec = NULL;
        double* tmp_local_start_sec = NULL;
        double* tmp_local_end_sec = NULL;

        // use global timings if available
        if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
          // gather measurement results
          int* local_errorcodes = sync_module->get_errorcodes();

          MPI_Gather(local_errorcodes, job_p->n_rep, MPI_INT,
                    sync_errorcodes, job_p->n_rep, MPI_INT, 0, MPI_COMM_WORLD);

          if (my_rank == OUTPUT_ROOT_PROC) {
            global_start_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));
            global_end_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));
          }

          tmp_local_start_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));
          tmp_local_end_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));
          for (i = 0; i < job_p->n_rep; i++) {
            tmp_local_start_sec[i] = clock_sync_module->get_global_time(job_p->tstart_sec[i]);
            tmp_local_end_sec[i] = clock_sync_module->get_global_time(job_p->tend_sec[i]);
          }
          MPI_Gather(tmp_local_start_sec, job_p->n_rep, MPI_DOUBLE, global_start_sec,
            job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

          MPI_Gather(tmp_local_end_sec, job_p->n_rep, MPI_DOUBLE, global_end_sec, job_p->n_rep,
                MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

          free(tmp_local_start_sec);
          free(tmp_local_end_sec);

          if (my_rank == OUTPUT_ROOT_PROC) {
              int proc_id;

              for (proc_id = 0; proc_id < np; proc_id++) {
                  for (i = 0; i < job_p->n_rep; i++) {
                      maxRuntimes_sec[proc_id * job_p->n_rep + i] = global_end_sec[proc_id * job_p->n_rep + i] -
                              global_start_sec[proc_id * job_p->n_rep+ i];
                  }
              }

              free(global_start_sec);
              free(global_end_sec);
          }

        } else {      // use local times
          if (my_rank == OUTPUT_ROOT_PROC) {
              local_start_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));
              local_end_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));
          }

          // gather local measurement results
          MPI_Gather(job_p->tstart_sec, job_p->n_rep, MPI_DOUBLE, local_start_sec,
              job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

          MPI_Gather(job_p->tend_sec, job_p->n_rep, MPI_DOUBLE, local_end_sec, job_p->n_rep,
                  MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);


          if (my_rank == OUTPUT_ROOT_PROC) {
            int proc_id;

            for (proc_id = 0; proc_id < np; proc_id++) {
                for (i = 0; i < job_p->n_rep; i++) {
                    maxRuntimes_sec[proc_id * job_p->n_rep + i] = local_end_sec[proc_id * job_p->n_rep + i] -
                        local_start_sec[proc_id * job_p->n_rep+ i];
                }
            }

            free(local_start_sec);
            free(local_end_sec);
          }
        }

        n_results = np;
    }



    if (my_rank == OUTPUT_ROOT_PROC) {
        long nreps;

        for (proc = 0; proc < n_results; proc++) {
            double* current_proc_runtimes;

            nreps = 0;
            current_proc_runtimes = maxRuntimes_sec + (proc * job_p->n_rep);

            // remove measurements with out-of-window errors
            if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
              int* current_error_codes;
              current_error_codes = sync_errorcodes + (proc * job_p->n_rep);

              for (i = 0; i < job_p->n_rep; i++) {
                if (current_error_codes[i] == 0) {
                    if (nreps < i) {
                        current_proc_runtimes[nreps] = current_proc_runtimes[i];
                    }
                    nreps++;
                }
              }
            }
            else {
            nreps = job_p->n_rep;
            }

            gsl_sort(current_proc_runtimes, 1, nreps);

            for (j=0; j<job_p->n_user_svars; j++) {
                fprintf(f, "%30s ", job_p->user_svars[j]);
            }
            for (j=0; j<job_p->n_user_ivars; j++) {
                fprintf(f, "%10d ", job_p->user_ivars[j]);
            }

            if (strcmp(job_p->timertype, "all") != 0) {
                fprintf(f, "%20s %7s %10ld %10ld ", job_p->timername, job_p->timertype, job_p->n_rep, nreps);
            }
            else {
                fprintf(f, "%20s %4d %10ld %10ld ", job_p->timername, proc, job_p->n_rep, nreps);
            }

            if (output_info_p->print_summary_methods > 0) {
              int i;
              for (i=0; i<reprompib_get_number_summary_methods(); i++) {
                summary_method_info_t* s = reprompib_get_summary_method(i);

                if (output_info_p->print_summary_methods & s->mask) {
                  double value = 0;

                  if (strcmp(s->name, "mean") == 0) {
                    value = gsl_stats_mean(maxRuntimes_sec, 1, nreps);
                  }
                  else if (strcmp(s->name, "median") == 0) {
                    value = gsl_stats_quantile_from_sorted_data (maxRuntimes_sec, 1, nreps, 0.5);
                  }
                  else if (strcmp(s->name, "min") == 0) {
                    if (nreps > 0) {
                      value = maxRuntimes_sec[0];
                    }
                  }
                  else if (strcmp(s->name, "max") == 0) {
                    if (nreps > 0) {
                      value = maxRuntimes_sec[nreps-1];
                    }
                  } else if (strcmp(s->name, "var") == 0) {
                    if (nreps > 0) {
                      value = gsl_stats_variance(maxRuntimes_sec, 1, nreps);
                    }
                  } else if (strcmp(s->name, "stddev") == 0) {
                    if (nreps > 0) {
                      value = gsl_stats_sd(maxRuntimes_sec, 1, nreps);
                    }
                  }
                  fprintf(f, "  %.10f ", value);
                }
              }
            }
            fprintf(f, "\n");
        }


        if (sync_module->procsync == REPROMPI_PROCSYNC_WIN) {
          free(sync_errorcodes);
        }
        free(maxRuntimes_sec);
    }
}






