
#include <vector>
#include "pgdata_comparer.h"

PGDataComparer::PGDataComparer(std::string mpi_coll_name, int nnodes, int ppn) :
mpi_coll_name(mpi_coll_name), nnodes(nnodes), ppn(ppn)
{}

void PGDataComparer::add_dataframe(std::string mockup_name, PGData *data) {
  mockup2data.insert( {mockup_name, data} );
}

void PGDataComparer::add_data(std::unordered_map<std::string, PGData *> data) {
  mockup2data = data;
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

PGDataResults::PGDataResults(std::string mpi_name, std::vector <std::string> col_names) :
    mpi_name(mpi_name), col_names(col_names) {
}

void PGDataResults::add_row(std::unordered_map <std::string, std::string> &row_map) {
  for (auto &rows: row_map) {
    col_value_map[rows.first].push_back(rows.second);
  }
}

void PGDataResults::set_col_widths(std::vector<int> widths) {
  col_widths = widths;
}

std::string PGDataResults::get_mpi_name() {
  return mpi_name;
}

std::vector<std::string> PGDataResults::get_col_names() {
  return col_names;
}

int PGDataResults::get_col_width(int index) {
  return col_widths.at(index);
}

std::unordered_map<std::string, std::vector<std::string>> PGDataResults::get_col_value_map() {
  return col_value_map;
}

int PGDataResults::get_col_size() {
  return col_value_map.at(col_names[0]).size();
}

std::vector<std::string> PGDataResults::get_values_for_col_name(std::string key) {
  return col_value_map.at(key);
}