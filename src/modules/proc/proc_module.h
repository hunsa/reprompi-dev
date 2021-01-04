//
// Created by niklas on 11.11.20.
//

#ifndef REPROMPI_PROC_MODULE_H
#define REPROMPI_PROC_MODULE_H

#ifdef PROC_MODULE

void init_proc_module(long nrep);

void save_process_prev_times();

void save_process_records(long repetition_id);

void print_process_records();

#else

#define init_proc_module(nrep)

#define save_process_prev_times()

#define save_process_records(repetition_id)

#define print_process_records()

#endif //PROC_MODULE

#endif //REPROMPI_PROC_MODULE_H
