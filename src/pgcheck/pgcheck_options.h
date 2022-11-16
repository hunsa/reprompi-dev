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
  std::string input_file = "";
  std::string output_directory = "./output/";
  bool detailed = false;
  bool csv = false;
  bool verbose = false;
  int comparer_type = 3;

public:

  PGCheckOptions(int argc, char *argv[]);

  void print_usage(char *command);

  std::string get_input_file();
  std::string get_output_directory();
  bool get_print_detailed_output();
  bool get_print_to_csv();
  bool get_verbose();
  int get_comparer_type();

};

#endif //REPROMPI_SRC_PGCHECK_PGCHECK_OPTIONS_H
