
#include <vector>
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
mpi_coll_name(mpi_coll_name), nnodes(nnodes), ppn(ppn)
{}

void PGDataComparer::add_dataframe(std::string mockup_name, PGData *data) {
  mockup2data.insert( {mockup_name, data} );
}

/**
   * adds the mean time for MPI_Barrier in seconds
   */
void PGDataComparer::set_barrier_time(double time_s) {
  barrier_time_s = time_s;
}

/**
 *
 * @return saved MPI_Barrier time in seconds
 */
double PGDataComparer::get_barrier_time() {
  return barrier_time_s;
}

/**
 *
 * @return true if barrier time has been set before
 */
bool PGDataComparer::has_barrier_time() {
  return barrier_time_s != -1.0;
}