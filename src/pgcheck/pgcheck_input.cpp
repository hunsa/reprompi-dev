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

#include "pgcheck_input.h"

PGInput::PGInput(std::string input_file_name) {
  std::ifstream ins(input_file_name);
  if (!ins.good()) {
    std::cerr << "Error: input file '" << input_file_name << "' cannot be read\n" << std::endl;
    exit(-1);
  }

  char buf[512];

  while (ins.getline(buf, 512)) {
    std::string line(buf);
    line = trim(line);
    if (line.size() > 0 && line.at(0) == '#') {
      // ignore comments '#'
      continue;
    }

    // now split the string into MPI call name + reprompi parameters
    // MPI_Bcast --msizes-list=4 --nrep=100
    int whitespace_pos = line.find(' ');
    std::string mpi_call_name = line.substr(0, whitespace_pos);
    std::string call_options_s = line.substr(whitespace_pos + 1, line.size() - whitespace_pos - 1);
    assert(mpi_call_name.size() > 0);
    assert(call_options_s.size() > 0);
    // std::cout << "'" << mpi_call_name << "'" << std::endl;
    // std::cout << "'" << call_options_s << "'" << std::endl;
    mpi_collectives.push_back(mpi_call_name);
    call_options.push_back(call_options_s);
  }
}

int PGInput::get_number_of_test_cases() {
  return mpi_collectives.size();
}

std::string PGInput::get_mpi_collective_for_case_id(int case_id) {
  assert(case_id >= 0 && case_id < static_cast<int>(mpi_collectives.size()));
  return mpi_collectives.at(case_id);
}

std::string PGInput::get_call_options_for_case_id(int case_id) {
  assert(case_id >= 0 && case_id < static_cast<int>(call_options.size()));
  return call_options.at(case_id);
}
