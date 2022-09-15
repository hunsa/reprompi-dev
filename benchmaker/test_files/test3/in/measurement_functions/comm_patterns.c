#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
//@ add_includes


char basetype[] = "MPI_INT";

void alltoall_pattern(int n_procs) {
  int i, j, msize;
  void *send_buffer, *recv_buffer;
  int rank;
  char* meas_functions[] = {"MPI_Alltoall" };
  int msize_list[] = {1, 64, 1024};
  int n_msizes = 3;
  char* meas_func;

  //@ declare_variables

  //@ initialize_timestamps t1
  //@ initialize_timestamps t2


  //@ global basetype=basetype

  for (i = 0; i < n_msizes; i++) {
    msize = msize_list[i];
    //@ set testtype="MPI_Alltoall"

    send_buffer = malloc(n_procs * msize);
    recv_buffer = malloc(n_procs * msize);

    //@ start_measurement_loop
    //@ measure_timestamp t1
    MPI_Alltoall(send_buffer, msize, MPI_BYTE, recv_buffer,
                 msize, MPI_BYTE, MPI_COMM_WORLD);

    //@ measure_timestamp t2
    //@stop_measurement_loop

    //DISABLE@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max testtype=testtype count=i

    //@ print_runtime_array name=runtime_per_process end_time=t2 start_time=t1 type=all testtype=testtype msize=i

    free(send_buffer);
    send_buffer = NULL;
    free(recv_buffer);
    recv_buffer = NULL;
  }

    //@cleanup_variables

}
