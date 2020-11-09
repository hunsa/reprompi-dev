//
// Created by niklas on 05.11.20.
//

#include "roth_tracing_module.h"


#ifdef ROTH_TRACING
#include <mpi.h>

void print_roth_tracing(int my_rank, int procs, long nrep, int root) {
    double *time_metric_receive_buffer = NULL;
    int *counter_metric_receive_buffer = NULL;
    long number_of_time_metrics_per_rank = nrep * get_number_of_time_metrics();
    long number_of_counter_metrics_per_rank = nrep * get_number_of_counter_metrics();
    if (my_rank == root) {
        time_metric_receive_buffer = malloc(procs * number_of_time_metrics_per_rank * sizeof(double));
        counter_metric_receive_buffer = malloc(procs * number_of_counter_metrics_per_rank * sizeof(int));
        printf("# Number of roth_tracing_time_records: %ld\n", number_of_time_metrics_per_rank * procs);
        printf("# Number of roth_tracing_counter_records: %ld\n", number_of_counter_metrics_per_rank * procs);
        fflush(stdout);
    }

    double *time_data = roth_tracing_get_time_data();
    int *counter_data = roth_tracing_get_counter_data();

    MPI_Gather(time_data, (int) number_of_time_metrics_per_rank, MPI_DOUBLE,
               time_metric_receive_buffer, (int) number_of_time_metrics_per_rank, MPI_DOUBLE,
               root, MPI_COMM_WORLD);
    MPI_Gather(counter_data, (int) number_of_counter_metrics_per_rank, MPI_INT,
               counter_metric_receive_buffer, (int) number_of_counter_metrics_per_rank, MPI_INT,
               root, MPI_COMM_WORLD);
    if (my_rank == root) {
        printf("nrep,rank,metric_name,value\n");
        for (long rank = 0; rank < procs; rank++) {
            long rank_start_index_time = rank * number_of_time_metrics_per_rank;
            long rank_start_index_counter = rank * number_of_counter_metrics_per_rank;
            for (long i = 0; i < nrep; i++) {
                long nrep_start_index_time = rank_start_index_time + i * get_number_of_time_metrics();
                for (time_metric metric_id = 0; metric_id < get_number_of_time_metrics(); metric_id++) {
                    long metric_index = nrep_start_index_time + metric_id;
                    // printf("# id: %ld\n", data_id);
                    printf("%ld,%ld,%s,%f\n", i, rank, get_time_metric_name(metric_id),
                           time_metric_receive_buffer[metric_index]);
                    fflush(stdout);
                }
                long nrep_start_index_counter = rank_start_index_counter + i * get_number_of_counter_metrics();
                for (counter_metric metric_id = 0; metric_id < get_number_of_counter_metrics(); metric_id++) {
                    long metric_index = nrep_start_index_counter + metric_id;
                    // printf("# id: %ld\n", data_id);
                    printf("%ld,%ld,%s,%d\n", i, rank, get_counter_metric_name(metric_id),
                           counter_metric_receive_buffer[metric_index]);
                    fflush(stdout);
                }
            }
        }
    }
}
#endif
