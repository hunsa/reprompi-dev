//
// Created by Max on 11/16/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H
#define REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H

#include <getopt.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <iomanip>
#include <sys/stat.h>

class PGCheckOptions {

protected:
  bool merge_coll_tables = false;    // write additional file containing merged results
  bool csv = false;                  // write results in csv format to file
  bool verbose = false;              // write information and results to console
  int comparer_type = 3;             // default is grouped t-test
  std::string input_file = "";
  std::string output_directory = "";
  std::string config_message = "";

public:
  PGCheckOptions() = default;
  bool get_merge_coll_tables();
  bool get_print_to_csv();
  bool get_verbose();
  bool get_csv();
  int get_comparer_type();
  std::string get_config_message();
  std::string get_input_file();
  std::string get_output_directory();
  /**
   * options are parsed
   * @return error code or warning message
   */
  std::string parse(int argc, char *argv[]);
};

#endif //REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H
