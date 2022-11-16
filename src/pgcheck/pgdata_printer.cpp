//
// Created by Max on 11/16/22.
//

#include "pgdata_printer.h"

PGDataPrinter::PGDataPrinter(int comparer_type, std::string output_directory, bool detailed, int nnodes, int ppn, bool verbose) :
    comparer_type(comparer_type), output_directory(output_directory), detailed(detailed), nnodes(nnodes), ppn(ppn), verbose(verbose) {}

void PGDataPrinter::add_dataframe_mockup(std::string mockup_name, PGData *data) {
  mockup2data.insert({mockup_name, data});
}

void PGDataPrinter::add_data_storage(std::string data) {
  data_storage.append(data);
}

/**
 * prints the PGData based on defined configurations
 */
int PGDataPrinter::print_collective() {

  PGDataComparer *comparer = ComparerFactory::create_comparer(comparer_type, mpi_coll_names.back(), nnodes, ppn);
  comparer->set_barrier_time(barrier_time_s);
  comparer->add_data(mockup2data);

  auto pgres = comparer->get_results();

  if(verbose || output_directory.empty()) {
    std::cout << pgres << std::endl;
  }

  if (!output_directory.empty()) {
    std::string file_name  = output_directory + mpi_coll_names.back() + ".txt";
    std::ofstream mockup_file(file_name);
    mockup_file << pgres << std::endl;
    mockup_file.close();
  }

  if (detailed) {
    add_data_storage(pgres);
  }

  return EXIT_SUCCESS;
}

void PGDataPrinter::println_to_cerr(std::string message) {
  std::cerr << message << std::endl;
}

void PGDataPrinter::println_to_cout(std::string message) {
  if (verbose) {
    std::cout << message << std::endl;
  }
}

void PGDataPrinter::print_summary() {
  if(detailed){
    std::string file_name  = output_directory + "MPI_Overview.txt";
    std::ofstream overview_file(file_name);
    overview_file << data_storage << std::endl;
    overview_file.close();
  }

  std::cout << "Files have been written to '" << output_directory << "'." << std::endl;
}

/**
 * adds the mean time for MPI_Barrier in seconds
 */
void PGDataPrinter::set_barrier_time(double time_s) {
  barrier_time_s = time_s;
}

/**
 * adds the coll name to mpi_coll_names vector
 */
void PGDataPrinter::add_mpi_coll_name(std::string mpi_coll_name) {
  mpi_coll_names.push_back(mpi_coll_name);
}
