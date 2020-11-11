//
// Created by niklas on 11.11.20.
//

#ifndef REPROMPI_PROC_MODULE_H
#define REPROMPI_PROC_MODULE_H

#ifdef PROC_MODULE

void init_proc_module(long nrep);

void save_process_prev_times();

void save_process_records(long repetition_id, int my_rank);

void print_process_records(int my_rank, int root, int procs);

#else

#define init_proc_module(nrep)

#define save_process_prev_times()

#define save_process_records(repetition_id, my_rank)

#define print_process_records(my_rank, root, procs)

#endif //PROC_MODULE

#endif //REPROMPI_PROC_MODULE_H
