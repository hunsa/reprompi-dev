//
// Created by Max on 11/16/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H
#define REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H

#include "pgdata.h"
#include "comparer/comparer_factory.h"
#include "comparer/raw/raw_comparer.h"
#include "pgcheck_options.h"
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <iostream>
#include <fstream>


class PGDataPrinter {
private:
  /**
   * @return table formatted for txt or console as string
   */
  std::string table_to_clear_string(PGDataTable data_result);

  /**
   * @return table formatted for csv file as string
   */
  std::string table_to_csv_string(PGDataTable data_result);

  /**
   * writes text to new file with filename
   */
  void write_string_to_file(std::string text, std::string filename);

  /**
   * adds the result from collective to table
   */
  void add_table_to_merged_table(PGDataTable data_table, size_t merge_table_id);

private:
  PGCheckOptions options;
  std::vector<PGDataTable> merged_table;
  std::vector <std::string> comparer_names = {"simple", "abs_runtime", "rel_runtime", "violation", "detailed_violation",
                                            "grouped_violation", "raw"};

public:
  PGDataPrinter() = default;

  /**
   * prints result from collective as txt or csv to file or console
   * @return 0 if print was successful
   */
  int print_collective(PGDataComparer *comparer, int comparer_type, size_t merge_table_id);

  /**
   * prints merged table as txt or csv to file or console
   * @return 0 if print was successful
   */
  int print_summary();

  /**
   * prints message to cout if verbose is enabled
   */
  void println_to_cout(std::string message);

  /**
   * prints warning message to cout, text color purple
   */
  void println_warning_to_cout(std::string message);

  /**
   * prints error message to cerr, text color red
   */
  void println_error_to_cerr(std::string message);

  /**
   * prints usage string
   */
  void print_usage(char *command);

  void set_options(PGCheckOptions &new_options);

};

#endif //REPROMPI_SRC_PGCHECK_PGDATA_PRINTER_H
