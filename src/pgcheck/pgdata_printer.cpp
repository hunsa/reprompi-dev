//
// Created by Max on 11/16/22.
//

#include "pgdata_printer.h"

PGDataPrinter::PGDataPrinter(int nnodes, int ppn, PGCheckOptions * options) :
    nnodes(nnodes), ppn(ppn), options(options) {}

std::string PGDataPrinter::table_to_clear_string(PGDataTable table) {
  std::stringstream res;
  std::vector<std::string> col_names = table.get_col_names();

  if(!table.get_mpi_name().empty()) {
    res << "MPI Collective: " << table.get_mpi_name() << "\n";
  } else {
    res << "MPI All Gathered Results " << "\n";
  }

  int idx = 0;
  for (auto iter = col_names.begin(); iter != col_names.end(); ++iter) {
    res << std::setw(table.get_col_width(idx++)) << *iter << " ";
  }
  res << "\n";
  int nb_rows = table.get_col_size();
  for (int i = 0; i < nb_rows; i++) {
    idx = 0;
    for (auto iter = col_names.begin(); iter != col_names.end(); ++iter) {
      std::vector<std::string> values = table.get_values_for_col_name(*iter);
      res << std::setw(table.get_col_width(idx++)) << values[i] << " ";
    }
    res << "\n";
  }
  return res.str();
}

std::string PGDataPrinter::table_to_csv_string(PGDataTable table) {
  std::stringstream res;
  std::string col_delimiter = ",";
  std::string row_delimiter = "\n";

  std::vector<std::string> col_names = table.get_col_names();
  for (auto iter = col_names.begin(); iter != col_names.end(); ++iter) {
    res << *iter;
    if (std::next(iter) != col_names.end()) {
      res << col_delimiter;
    }
  }
  res << row_delimiter;
  int nb_rows = table.get_col_size();
  for (int i = 0; i < nb_rows; i++) {

    for (auto iter = col_names.begin(); iter != col_names.end(); ++iter) {
      std::vector<std::string> values = table.get_values_for_col_name(*iter);
      res << values[i];
      if (std::next(iter) != col_names.end()) {
        res << col_delimiter;
      }
    }
    res << row_delimiter;
  }
  return res.str();
}

int PGDataPrinter::print_collective() {

  PGDataComparer *comparer = ComparerFactory::create_comparer(options->get_comparer_type(), mpi_coll_names.back(), nnodes, ppn);
  comparer->set_barrier_time(barrier_time_s);
  comparer->add_data(mockup2data);

  PGDataTable table_coll_res = comparer->get_results();
  std::string output_formatted = table_to_clear_string(table_coll_res);
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
    csv_file << table_to_csv_string(table_coll_res) << std::endl;
    csv_file.close();
  }

  if (options->get_merge_coll_tables()) {
    if (merged_table.get_col_names().empty()) {
      merged_table.set_col_names(table_coll_res.get_col_names());
      merged_table.set_col_widths(table_coll_res.get_col_widths());
    }
    merged_table.add_table(table_coll_res);
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
  if(options->get_merge_coll_tables()){

    std::string merged_table_string = table_to_clear_string(merged_table);

    if (options->get_verbose()) {
      std::cout << merged_table_string;
    }

    std::string file_name_txt  = options->get_output_directory() + "MPI_Results.txt";
    std::ofstream merged_file(file_name_txt);
    merged_file << merged_table_string << std::endl;
    merged_file.close();

    if(options->get_csv()) {
      std::string file_name_csv  = options->get_output_directory() + "MPI_Results.csv";
      std::ofstream merged_csv_file(file_name_csv);
      merged_csv_file << table_to_csv_string(merged_table) << std::endl;
      merged_csv_file.close();
    }
  }

  if (!options->get_output_directory().empty()) {
    std::cout << "Files have been written to '" << options->get_output_directory() << "'." << std::endl;
  }
}

void PGDataPrinter::add_dataframe_mockup(std::string mockup_name, PGData *data) {
  mockup2data.insert({mockup_name, data});
}

void PGDataPrinter::set_barrier_time(double time_s) {
  barrier_time_s = time_s;
}

void PGDataPrinter::add_mpi_coll_name(std::string mpi_coll_name) {
  mpi_coll_names.push_back(mpi_coll_name);
}
