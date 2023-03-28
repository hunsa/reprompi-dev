//
// Created by Max on 11/16/22.
//

#include "pgdata_printer.h"

int PGDataPrinter::print_collective(PGDataComparer *comparer, int comparer_type, size_t merge_table_id) {
  PGDataTable table_coll_res = comparer->get_results();
  std::string output_directory = options.get_output_directory();

  std::string filename = "";
  std::string folder_name;

  if (options.get_allow_mkdir()) {
    folder_name = output_directory + comparer_names.at(comparer_type) + "/";
    filename = folder_name + table_coll_res.get_mpi_name();
  } else {
    filename = output_directory + comparer_names.at(comparer_type) + "_" + table_coll_res.get_mpi_name();
  }

  // never print raw to cout
  if(typeid(*comparer).name() != typeid(RawComparer).name()) {
    std::string output_formatted = table_to_clear_string(table_coll_res);
    println_to_cout(output_formatted);
  } else {
    println_to_cout("Raw Runtime Data was written to file.");
  }

  char * folder_chars = const_cast<char*>(folder_name.c_str());
  if (!output_directory.empty()) {
    if (options.get_allow_mkdir()) {
      mkdir(folder_chars, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if (options.get_csv()) {
      write_string_to_file(table_to_csv_string(table_coll_res), filename + ".csv");
    }
  }

  if (options.get_merge_coll_tables()) {
    add_table_to_merged_table(table_coll_res, merge_table_id);
  }

  return EXIT_SUCCESS;
}

int PGDataPrinter::print_summary() {
  std::string output_directory = options.get_output_directory();
  if (options.get_merge_coll_tables()) {

    size_t merge_table_id = 0;

    for (auto table : merged_table) {
      std::string merged_table_string = table_to_clear_string(table);
      size_t comp_name = options.get_comparer_list().at(merge_table_id++);

      std::string merged_table_filename;
      std::string stats_filename;

      if (options.get_allow_mkdir()) {
        merged_table_filename = output_directory + comparer_names.at(comp_name) + "/Results";
        stats_filename = output_directory + comparer_names.at(comp_name) + "/Stats";
      } else {
        merged_table_filename = output_directory + comparer_names.at(comp_name) + "_Results";
        stats_filename = output_directory + comparer_names.at(comp_name) + "_Stats";
      }


      if(!output_directory.empty()) {
        write_string_to_file(merged_table_string, merged_table_filename + ".txt");

        if (options.get_csv()) {
          write_string_to_file(table_to_csv_string(table), merged_table_filename + ".csv");
        }
      }

      // print stats only for violation comparer
      if (comp_name > 2 && comp_name < 6) {
        println_separator_to_cout();
        println_to_cout("Violations Statistics for " + comparer_names.at(comp_name));

        std::string stats_clear_string = table_to_clear_string(table.get_violation_table());

        println_to_cout(stats_clear_string);

        if(!output_directory.empty()) {
          write_string_to_file(stats_clear_string, stats_filename + ".txt");

          if (options.get_csv()) {
            write_string_to_file(table_to_csv_string(table.get_violation_table()), stats_filename + ".csv");
          }
        }
      }
    }
  }

  println_separator_to_cout();
  if (!output_directory.empty()) {
    std::cout << "Files have been written to '" << options.get_output_directory() << "'." << std::endl;
  }

  return EXIT_SUCCESS;
}

std::string PGDataPrinter::table_to_clear_string(PGDataTable table) {
  const std::vector<std::string>& col_names = table.get_col_names();
  const int nb_rows = table.get_col_size();

  std::ostringstream res;
  res.exceptions(std::ios::failbit);
  res.rdbuf()->pubsetbuf(nullptr, 0);
  res.precision(10);
  res << std::fixed;
  res.exceptions(std::ios::goodbit);
  res.str().reserve(col_names.size() * 10 + nb_rows * col_names.size() * 20);

  if (!table.get_mpi_name().empty()) {
    res << "MPI Collective: " << table.get_mpi_name() << "\n";
  } else {
    res << "MPI All Results " << "\n";
  }

  int idx = 0;
  for (auto col_name = col_names.cbegin(); col_name != col_names.cend(); ++col_name) {
    res << std::left << std::setw(table.get_col_width(idx++)) << *col_name << " ";
  }
  res << "\n";

  for (int i = 0; i != nb_rows; ++i) {
    idx = 0;
    for (auto col_name = col_names.cbegin(); col_name != col_names.cend(); ++col_name) {
      res << std::left << std::setw(table.get_col_width(idx++)) << table.get_values_col_row(*col_name, i) << " ";
    }
    res << "\n";
  }
  return res.str();
}

std::string PGDataPrinter::table_to_csv_string(PGDataTable table) {
  const std::string col_delimiter = ",";
  const std::string row_delimiter = "\n";
  const auto& col_names = table.get_col_names();
  const int nb_rows = table.get_col_size();

  std::ostringstream res;
  res.exceptions(std::ios::failbit);
  res.rdbuf()->pubsetbuf(nullptr, 0);
  res.precision(10);
  res << std::fixed;
  res.exceptions(std::ios::goodbit);
  res.str().reserve(col_names.size() * 10 + nb_rows * col_names.size() * 20);

  for (auto col_name = col_names.cbegin(); col_name != col_names.cend(); ++col_name) {
    res << *col_name;
    if (std::next(col_name) != col_names.cend()) {
      res << col_delimiter;
    }
  }
  res << row_delimiter;

  for (int i = 0; i != nb_rows; ++i) {
    for (auto col_name = col_names.cbegin(); col_name != col_names.cend(); ++col_name) {
      res << table.get_values_col_row(*col_name, i);
      if (std::next(col_name) != col_names.cend()) {
        res << col_delimiter;
      }
    }
    if (i != nb_rows - 1) {
      res << row_delimiter;
    }
  }
  return res.str();
}

void PGDataPrinter::print_separator_to_cout() {
  if (options.get_verbose()) {
    char fill_char = '#';
    std::cout << std::left << std::setfill(fill_char) << std::setw(80) << "" << std::endl;
  }
}

void PGDataPrinter::println_separator_to_cout() {
  if (options.get_verbose()) {
    char fill_char = '#';
    std::cout << std::endl << std::left << std::setfill(fill_char) << std::setw(80) << "" << std::endl;
  }
}

void PGDataPrinter::println_to_cout(std::string message) {
  if (options.get_verbose()) {
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

void PGDataPrinter::add_table_to_merged_table(PGDataTable data_table, size_t merge_table_id) {
  if (merged_table.size() < (1 + merge_table_id)) {
    merged_table.push_back(PGDataTable());
  }
  if (merged_table.at(merge_table_id).get_col_names().empty()) {
    merged_table.at(merge_table_id).set_col_names(data_table.get_col_names());
    merged_table.at(merge_table_id).set_col_widths(data_table.get_col_widths());
  }
  merged_table.at(merge_table_id).add_table(data_table);
}

void PGDataPrinter::print_usage(char *command) {
  std::cout << "USAGE: " << std::string(command) << " -f input_file [options]" << std::endl << std::endl;
  std::cout << "OPTIONS:" << std::endl;
  std::cout << std::setw(36) << std::left << "  ?, -h, --help" << "Display this information." << std::endl;
  std::cout << std::setw(36) << std::left << "  -c, --comp-list={0|1|2|3|4|5|6}" << "Specify the comparer type (0=Simple|1=Absolute Median|2=Relative Median|3=Violation-Test|4=Detailed Violation-Test|5=Grouped Violation-Test|6=Raw Data)." << std::endl;
  std::cout << std::setw(36) << std::left << "  -t, --test {0|1|2}" << "Specify the test type (0=T-Test|1=Wilcoxon-Rank-Sum-Test|2=Wilcoxon-Mann-Whitney)." << std::endl;
  std::cout << std::setw(36) << std::left << "  -o, --output <path>" << "Specify an existing output folder." << std::endl;
  std::cout << std::setw(36) << std::left << "  -m, --merge" << "Additionally results of all collectives are merged into one table." << std::endl;
  std::cout << std::setw(36) << std::left << "  -d, --allow-mkdir" << "Allow PGChecker to generate folders in the specified output folder" << std::endl;
  std::cout << std::setw(36) << std::left << "  -s, --csv" << "Print results to .csv file. Output directory must be specified. The csv formatted table is never written to the console." << std::endl;
  std::cout << std::setw(36) << std::left << "  -v, --verbose" << "Print all information and results to console." << std::endl;
}

void PGDataPrinter::set_options(PGCheckOptions &new_options) {
  options = new_options;
}
