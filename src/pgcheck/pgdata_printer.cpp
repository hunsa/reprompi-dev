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

namespace fs = std::filesystem;

int PGDataPrinter::print_collective(std::unique_ptr<PGDataComparer>& comparer, int comparer_type, size_t merge_table_id) {
  PGDataTable table_coll_res = comparer->get_results();
  std::string output_directory = options.get_output_directory();

  fs::path comparer_dir = output_directory;
  comparer_dir /= CONSTANTS::COMPARER_NAMES.at(comparer_type);
  fs::create_directory(comparer_dir);

  fs::path out_txt_fname = comparer_dir / (table_coll_res.get_title() + ".txt");
  fs::path out_csv_fname = comparer_dir / (table_coll_res.get_title() + ".csv");

  std::string output_formatted = table_coll_res.to_string();
  std::string comp_clear_name = CONSTANTS::COMPARER_NAMES.at(comparer_type);
  std::transform(comp_clear_name.begin(), comp_clear_name.end(), comp_clear_name.begin(), ::toupper);

  // never print raw to cout
  if (typeid(*comparer).name() != typeid(RawComparer).name()) {
    Logger::EVAL_BLOCK("TABLE FOR " + comp_clear_name + " COMPARER", output_formatted);
  }

  Logger::INFO("Writing Data:           " + out_txt_fname.string());
  Logger::FILE(output_formatted, out_txt_fname.string());

  if (options.get_csv()) {
    Logger::INFO("Writing Data:           " + out_csv_fname.string());
    Logger::FILE(table_coll_res.to_csv_string(), out_csv_fname.string());
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
      std::string merged_table_string = table.to_string();
      size_t comp_name = options.get_comparer_list().at(merge_table_id++);

      fs::path outpath = options.get_output_directory();
      fs::path merged_table_dir = outpath / CONSTANTS::COMPARER_NAMES.at(comp_name) ;
      fs::path stats_dir        = outpath / CONSTANTS::COMPARER_NAMES.at(comp_name) ;

      fs::create_directory(merged_table_dir);
      fs::create_directory(stats_dir);

      fs::path merged_table_filename = merged_table_dir / "results.txt";
      fs::path merged_table_csv_filename = merged_table_dir / "results.csv";

      Logger::FILE(merged_table_string, merged_table_filename.string());
      Logger::INFO("Writing Data:           " + merged_table_filename.string());
      if (options.get_csv()) {
        Logger::FILE(table.to_csv_string(), merged_table_csv_filename.string());
        Logger::INFO("Writing Data:           " + merged_table_csv_filename.string());
      }

      // print stats only for violation comparer
      if (comp_name > 2 && comp_name < 6) {
        std::string stats_clear_string = table.get_violation_table().to_string();
        std::string comp_clear_name = CONSTANTS::COMPARER_NAMES.at(comp_name);
        std::transform(comp_clear_name.begin(), comp_clear_name.end(), comp_clear_name.begin(), ::toupper);
        Logger::EVAL_BLOCK("VIOLATION COUNT FOR " + comp_clear_name + " COMPARER", stats_clear_string);

        fs::path stats_filename = stats_dir / "stats.txt";
        fs::path stats_csv_filename = stats_dir / "stats.csv";

        Logger::INFO("Writing Data:           " + stats_filename.string());
        Logger::FILE(stats_clear_string, stats_filename);

        if (options.get_csv()) {
          Logger::INFO("Writing Data:           " + stats_csv_filename.string());
          Logger::FILE(table.get_violation_table().to_csv_string(), stats_csv_filename.string());
        }
      }
    }
  }

  if (!output_directory.empty()) {
    Logger::INFO("PGChecker Done:         output in " + options.get_output_directory());
  }

  return EXIT_SUCCESS;
}

void PGDataPrinter::add_table_to_merged_table(PGDataTable data_table, size_t merge_table_id) {
  if (merged_table.size() < (1 + merge_table_id)) {
    merged_table.push_back(PGDataTable());
  }
  if (merged_table.at(merge_table_id).get_column_names().empty()) {
    merged_table.at(merge_table_id).set_column_names(data_table.get_column_names());
    merged_table.at(merge_table_id).set_col_widths(data_table.get_col_widths());
  }
  merged_table.at(merge_table_id).add_table(data_table);
}

void PGDataPrinter::set_options(const PGCheckOptions &new_options) {
  options = new_options;
}
