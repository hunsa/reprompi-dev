//
// Created by Max on 11/16/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H
#define REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H

#include <getopt.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <sys/stat.h>

class PGCheckOptions {

protected:
  bool merge_coll_tables = false;
  bool csv = false;
  bool verbose = false;
  int comparer_type = 3;
  std::string input_file = "";
  std::string output_directory = "";
  std::string config_message = "";

public:
  PGCheckOptions(int argc, char *argv[]);
  void print_usage(char *command);
  bool get_merge_coll_tables();
  bool get_print_to_csv();
  bool get_verbose();
  bool get_csv();
  int get_comparer_type();
  std::string get_config_message();
  std::string get_input_file();
  std::string get_output_directory();
};

#endif //REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H
