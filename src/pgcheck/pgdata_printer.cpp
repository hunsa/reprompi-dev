/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#include "pgdata_printer.h"

int PGDataPrinter::print_collective(PGDataComparer *comparer, int comparer_type, size_t merge_table_id) {
  PGDataTable table_coll_res = comparer->get_results();
  std::string output_directory = options.get_output_directory();

  std::string filename = "";
  std::string folder_name;

  if (options.get_allow_mkdir()) {
    folder_name = output_directory + CONSTANTS::COMPARER_NAMES.at(comparer_type) + "/";
    filename = folder_name + table_coll_res.get_mpi_name();
  } else {
    filename = output_directory + CONSTANTS::COMPARER_NAMES.at(comparer_type) + "_" + table_coll_res.get_mpi_name();
  }

  std::string output_formatted = table_to_clear_string(table_coll_res);
  std::string comp_clear_name = CONSTANTS::COMPARER_NAMES.at(comparer_type);
  std::transform(comp_clear_name.begin(), comp_clear_name.end(), comp_clear_name.begin(), ::toupper);

  // never print raw to cout
  if (typeid(*comparer).name() != typeid(RawComparer).name()) {
    print_evaluation_to_cout(output_formatted, "TABLE FOR " + comp_clear_name + " COMPARER");
  }

  char *folder_chars = const_cast<char *>(folder_name.c_str());
  if (!output_directory.empty()) {
    if (options.get_allow_mkdir()) {
      mkdir(folder_chars, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    println_info_to_cout("Writing Data:           " + filename + ".txt");
    write_string_to_file(output_formatted, filename + ".txt");

    if (options.get_csv()) {
      println_info_to_cout("Writing Data:           " + filename + ".csv");
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
        merged_table_filename = output_directory + CONSTANTS::COMPARER_NAMES.at(comp_name) + "/Results";
        stats_filename = output_directory + CONSTANTS::COMPARER_NAMES.at(comp_name) + "/Stats";
      } else {
        merged_table_filename = output_directory + CONSTANTS::COMPARER_NAMES.at(comp_name) + "_Results";
        stats_filename = output_directory + CONSTANTS::COMPARER_NAMES.at(comp_name) + "_Stats";
      }

      if (!output_directory.empty()) {
        write_string_to_file(merged_table_string, merged_table_filename + ".txt");
        println_info_to_cout("Writing Data:           " + merged_table_filename + ".txt");
        if (options.get_csv()) {
          write_string_to_file(table_to_csv_string(table), merged_table_filename + ".csv");
          println_info_to_cout("Writing Data:           " + merged_table_filename + ".csv");
        }
      }

      // print stats only for violation comparer
      if (comp_name > 2 && comp_name < 6) {
        std::string stats_clear_string = table_to_clear_string(table.get_violation_table());
        std::string comp_clear_name = CONSTANTS::COMPARER_NAMES.at(comp_name);
        std::transform(comp_clear_name.begin(), comp_clear_name.end(), comp_clear_name.begin(), ::toupper);
        print_evaluation_to_cout(stats_clear_string, "VIOLATION COUNT FOR " + comp_clear_name + " COMPARER");
        if (!output_directory.empty()) {
          println_info_to_cout("Writing Data:           " + stats_filename + ".txt");
          write_string_to_file(stats_clear_string, stats_filename + ".txt");

          if (options.get_csv()) {
            println_info_to_cout("Writing Data:           " + stats_filename + ".csv");
            write_string_to_file(table_to_csv_string(table.get_violation_table()), stats_filename + ".csv");
          }
        }
      }
    }
  }

  if (!output_directory.empty()) {
    println_info_to_cout("PGChecker Done:         output in " + options.get_output_directory());
  }

  return EXIT_SUCCESS;
}

std::string PGDataPrinter::table_to_clear_string(PGDataTable table) {
  const std::vector <std::string> &col_names = table.get_col_names();
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
  const auto &col_names = table.get_col_names();
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

void PGDataPrinter::print_evaluation_to_cout(std::string message, std::string heading) {
  std::cout << std::endl << "\033[33m" << "[EVALUATION | " << heading << "]       " << "\033[0m" << std::endl;
  std::cout << message;
  std::cout << "\033[33m" << "[EVALUATION | " << heading << "] " << "\033[0m" << std::endl << std::endl;
}

void PGDataPrinter::println_info_to_cout(std::string message) {
  std::cout << "\033[34m" << "[INFO]    " << "\033[0m" << message << std::endl;
}

void PGDataPrinter::println_warning_to_cout(std::string message) {
  std::cout << "\033[35m" << "[WARNING] " << "\033[0m" << message << std::endl;
}

void PGDataPrinter::println_error_to_cerr(std::string message) {
  std::cerr << "\033[31m" << "[ERROR]   " << "\033[0m" << message << std::endl;
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
  std::cout << std::setw(36) << std::left << "  -c, --comp-list={0|1|2|3|4|5|6}"
            << "Specify the comparer type ("
            << "0=Simple|"
            << "1=Absolute Median|"
            << "2=Relative Median|"
            << "3=Violation-Test|"
            << "4=Detailed Violation-Test|"
            << "5=Grouped Violation-Test|"
            << "6=Raw Data)."
            << std::endl;
  std::cout << std::setw(36) << std::left << "  -t, --test {0|1|2}"
            << "Specify the test type (0=T-Test|1=Wilcoxon-Rank-Sum-Test|2=Wilcoxon-Mann-Whitney)." << std::endl;
  std::cout << std::setw(36) << std::left << "  -o, --output <path>" << "Specify an existing output folder."
            << std::endl;
  std::cout << std::setw(36) << std::left << "  -m, --merge"
            << "Additionally results of all collectives are merged into one table." << std::endl;
  std::cout << std::setw(36) << std::left << "  -d, --allow-mkdir"
            << "Allow PGChecker to generate folders in the specified output folder" << std::endl;
  std::cout << std::setw(36) << std::left << "  -s, --csv"
            << "Print results to .csv file. Output directory must be specified. "
            << "The csv formatted table is never written to the console."
            << std::endl;
  std::cout << std::setw(36) << std::left << "  -v, --verbose" << "Print all information and results to console."
            << std::endl;
}

void PGDataPrinter::set_options(const PGCheckOptions &new_options) {
  options = new_options;
}
