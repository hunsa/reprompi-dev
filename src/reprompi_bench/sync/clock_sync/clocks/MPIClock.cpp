
#include <mpi.h>
#include "MPIClock.h"

MPIClock::MPIClock() {

}

MPIClock::~MPIClock() {

}

double MPIClock::get_time(void) {
  return MPI_Wtime();
};

bool MPIClock::is_base_clock() {
  return true;
}
