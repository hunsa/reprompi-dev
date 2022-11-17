//
// Created by Max on 11/16/22.
//

#include "pgdata_printer.h"

PGDataPrinter::PGDataPrinter(PGCheckOptions * options, int nnodes, int ppn) :
    options(options), nnodes(nnodes), ppn(ppn) {}

void PGDataPrinter::add_dataframe_mockup(std::string mockup_name, PGData *data) {
  mockup2data.insert({mockup_name, data});
}

void PGDataPrinter::add_data_storage(std::string data) {
  /*data_storage.append(data);*/
}

std::string PGDataPrinter::pgdata_to_string(PGDataTable data_result) {
  std::stringstream res;
  res << "MPI Collective: " << data_result.get_mpi_name() << "\n";
  int idx = 0;
  for (auto &colname: data_result.get_col_names()) {
    res << std::setw(data_result.get_col_width(idx++)) << colname << " ";
  }
  res << "\n";
  int nb_rows = data_result.get_col_size();
  for (int i = 0; i < nb_rows; i++) {
    idx = 0;
    for (auto &col_name: data_result.get_col_names()) {
      std::vector<std::string> values = data_result.get_values_for_col_name(col_name);
      res << std::setw(data_result.get_col_width(idx++)) << values[i] << " ";
    }
    res << "\n";
  }
  return res.str();
}

std::string PGDataPrinter::pgdata_to_csv_string(PGDataTable data_result) {
  std::stringstream res;
  std::string col_delimiter = ",";
  std::string row_delimiter = "\n";

  std::vector<std::string> col_names = data_result.get_col_names();
  for (auto iter = col_names.begin(); iter != col_names.end(); ++iter) {
    res << *iter;
    if (std::next(iter) != col_names.end()) {
      res << col_delimiter;
    }
  }
  res << row_delimiter;
  int nb_rows = data_result.get_col_size();
  for (int i = 0; i < nb_rows; i++) {

    for (auto iter = col_names.begin(); iter != col_names.end(); ++iter) {
      std::vector<std::string> values = data_result.get_values_for_col_name(*iter);
      res << values[i];
      if (std::next(iter) != col_names.end()) {
        res << col_delimiter;
      }
    }
    res << row_delimiter;
  }
  return res.str();
}

/**
 * prints the PGData based on defined options
 */
int PGDataPrinter::print_collective() {

  PGDataComparer *comparer = ComparerFactory::create_comparer(options->get_comparer_type(), mpi_coll_names.back(), nnodes, ppn);
  comparer->set_barrier_time(barrier_time_s);
  comparer->add_data(mockup2data);

  PGDataTable pgres = comparer->get_results();
  std::string output_formatted = pgdata_to_string(pgres);

  std::string output_directory = options->get_output_directory();

  if(options->get_verbose() || output_directory.empty()) {
    std::cout << output_formatted << std::endl;
  }

  if (!output_directory.empty()) {
    std::string file_name  = output_directory + mpi_coll_names.back() + ".txt";
    std::ofstream mockup_file(file_name);
    mockup_file << output_formatted << std::endl;
    mockup_file.close();
  }

  if (options->get_csv() && !output_directory.empty()) {
    std::string csv_file_name  = output_directory + mpi_coll_names.back() + ".csv";
    std::ofstream csv_file(csv_file_name);
    csv_file << pgdata_to_csv_string(pgres) << std::endl;
    csv_file.close();
  }

  return EXIT_SUCCESS;
}

void PGDataPrinter::println_to_cerr(std::string message) {
  std::cerr << message << std::endl;
}

void PGDataPrinter::println_to_cout(std::string message) {
  if (options->get_verbose()) {
    std::cout << message << std::endl;
  }
}

void PGDataPrinter::print_summary() {
  /*
  if(detailed){
    std::string file_name  = output_directory + "MPI_Overview.txt";
    std::ofstream overview_file(file_name);
    overview_file << data_storage << std::endl;
    overview_file.close();
  }
   */

  std::cout << "Files have been written to '" << options->get_output_directory() << "'." << std::endl;
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
