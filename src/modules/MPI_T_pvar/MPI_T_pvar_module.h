//
// Created by niklas on 10.11.20.
//

#ifndef REPROMPI_MPI_T_PVAR_MODULE_H
#define REPROMPI_MPI_T_PVAR_MODULE_H

#ifdef MPI_T_PVARS
void init_MPI_T_pvars(long nrep);

void start_MPI_T_pvars();

void save_MPI_T_pvars(long repetition_id, int my_rank);

void print_pvars(int my_rank, int root, int procs);

void clean_up_MPI_T_pvars();

#else

#define init_MPI_T_pvars(nrep)

#define start_MPI_T_pvars()

#define save_MPI_T_pvars(repetition_id, my_rank)

#define print_pvars(my_rank, root, procs)

#define clean_up_MPI_T_pvars();

#endif //MPI_T_PVARS

#endif //REPROMPI_MPI_T_PVAR_MODULE_H
