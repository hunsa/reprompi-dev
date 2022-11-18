
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
  nnodes(nnodes), ppn(ppn), mpi_coll_name(mpi_coll_name)
{}

bool PGDataComparer::has_barrier_time() {
  return barrier_time_s != -1.0;
}

double PGDataComparer::get_barrier_time() {
  return barrier_time_s;
}

void PGDataComparer::add_data(std::unordered_map<std::string, PGData *> data) {
  mockup2data = data;
}

void PGDataComparer::set_barrier_time(double time_s) {
  barrier_time_s = time_s;
}