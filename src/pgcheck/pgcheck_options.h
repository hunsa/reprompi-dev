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
#include <vector>
#include <sys/stat.h>

class PGCheckOptions {

protected:
  bool merge_coll_tables = false;       // write additional file containing merged results
  bool csv = false;                     // write results in csv format to file
  bool verbose = false;                 // write information and results to console
  bool allow_mkdir = false;             // allow pgchecker to make directories
  std::vector<int> comparer_list = {6}; // raw data is always written
  int test_type = 0;                    // default is t-test
  std::string input_file = "";
  std::string output_directory = "";
  std::string config_message = "";

public:
  PGCheckOptions() = default;
  bool get_merge_coll_tables();
  bool get_print_to_csv();
  bool get_verbose();
  bool get_allow_mkdir();
  bool get_csv();
  std::vector<int> get_comparer_list();
  int get_test_type();
  std::string get_config_message();
  std::string get_input_file();
  std::string get_output_directory();
  /**
   * options are parsed
   * @return error code or warning message
   */
  int parse(int argc, char *argv[]);
};

#endif //REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H
