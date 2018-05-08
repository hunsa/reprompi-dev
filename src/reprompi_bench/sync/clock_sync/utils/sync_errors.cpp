
#include <cstdio>
#include <cstdlib>
#include <mpi.h>

#include "sync_errors.hpp"

void exit_on_sync_lib_error(const char* error_str) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    fprintf(stderr, "\nERROR: %s\n\n", error_str);
  }
  MPI_Finalize();
  exit(1);
}

