
#ifndef REPROMPI_DEV_SRC_PGCHECK_PGCHECK_INPUT_H
#define REPROMPI_DEV_SRC_PGCHECK_PGCHECK_INPUT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include "utils/string_utils.h"

class PGInput {

private:
  std::vector<std::string> mpi_collectives;
  std::vector<std::string> call_options;

public:
  PGInput(std::string input_file_name);
  int get_number_of_test_cases();
  std::string get_mpi_collective_for_case_id(int case_id);
  std::string get_call_options_for_case_id(int case_id);
};

#endif //REPROMPI_DEV_SRC_PGCHECK_PGCHECK_INPUT_H
