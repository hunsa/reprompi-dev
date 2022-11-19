//
// Created by Max on 11/16/22.
//

#include "pgdata_printer.h"

PGDataPrinter::PGDataPrinter(PGCheckOptions *options) : options(options) {}

int PGDataPrinter::print_collective(PGDataComparer *comparer) {
  PGDataTable table_coll_res = comparer->get_results();
  std::string output_formatted = table_to_clear_string(table_coll_res);
  std::string output_directory = options->get_output_directory();
  std::string filename = output_directory + table_coll_res.get_mpi_name();

  println_to_cout(output_formatted);

  if (!output_directory.empty()) {
    write_string_to_file(output_formatted, filename + ".txt");

    if (options->get_csv()) {
      write_string_to_file(table_to_csv_string(table_coll_res), filename + ".csv");
    }
  }

  if (options->get_merge_coll_tables()) {
    add_table_to_merged_table(table_coll_res);
  }

  return EXIT_SUCCESS;
}

int PGDataPrinter::print_summary() {
  std::string output_directory = options->get_output_directory();
  if (options->get_merge_coll_tables()) {
    std::string merged_table_string = table_to_clear_string(merged_table);
    std::string filename = output_directory + "MPI_Results";

    println_to_cout(merged_table_string);

    if(!output_directory.empty()) {
      write_string_to_file(merged_table_string, filename + ".txt");

      if (options->get_csv()) {
        write_string_to_file(table_to_csv_string(merged_table), filename + ".csv");
      }
    }
  }

  if (!output_directory.empty()) {
    std::cout << "Files have been written to '" << options->get_output_directory() << "'." << std::endl;
  }

  return EXIT_SUCCESS;
}

std::string PGDataPrinter::table_to_clear_string(PGDataTable table) {
  std::stringstream res;
  std::vector <std::string> col_names = table.get_col_names();

  if (!table.get_mpi_name().empty()) {
    res << "MPI Collective: " << table.get_mpi_name() << "\n";
  } else {
    res << "MPI All Results " << "\n";
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
      std::vector <std::string> values = table.get_values_for_col_name(*iter);
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

  std::vector <std::string> col_names = table.get_col_names();
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
      std::vector <std::string> values = table.get_values_for_col_name(*iter);
      res << values[i];
      if (std::next(iter) != col_names.end()) {
        res << col_delimiter;
      }
    }
    res << row_delimiter;
  }
  return res.str();
}

void PGDataPrinter::println_to_cout(std::string message) {
  if (options->get_verbose()) {
    std::cout << message << std::endl;
  }
}

void PGDataPrinter::println_warning_to_cout(std::string message) {
  std::cout << "\033[35m" << "Warning: " << message << "\033[0m" << std::endl;
}

void PGDataPrinter::println_error_to_cerr(std::string message) {
  std::cerr << "\033[31m" << "Error: " << message << "\033[0m" << std::endl;
}

void PGDataPrinter::write_string_to_file(std::string text, std::string filename) {
  std::ofstream file(filename);
  file << text << std::endl;
  file.close();
}

void PGDataPrinter::add_table_to_merged_table(PGDataTable data_table) {
  if (merged_table.get_col_names().empty()) {
    merged_table.set_col_names(data_table.get_col_names());
    merged_table.set_col_widths(data_table.get_col_widths());
  }
  merged_table.add_table(data_table);
}

void PGDataPrinter::print_usage(char *command) {
  std::cout << "USAGE: " << std::string(command) << " -f input_file [options]" << std::endl << std::endl;
  std::cout << "OPTIONS:" << std::endl;
  std::cout << std::setw(28) << std::left << "  ?, -h, --help" << "Display this information." << std::endl;
  std::cout << std::setw(28) << std::left << "  -c, --comparer {0|1|2|3}" << "Specify the comparer type (0=Simple; 1=Detailed; 2=T-Test; 3=Grouped T-Test)." << std::endl;
  std::cout << std::setw(28) << std::left << "  -o, --output <path>" << "Specify an existing output folder." << std::endl;
  std::cout << std::setw(28) << std::left << "  -m, --merge" << "Additionally results of all collectives are merged into one table." << std::endl;
  std::cout << std::setw(28) << std::left << "  -s, --csv" << "Print results to .csv file. Output directory must be specified. The csv formatted table is never written to the console." << std::endl;
  std::cout << std::setw(28) << std::left << "  -v, --verbose" << "Print all information and results to console." << std::endl;
}
