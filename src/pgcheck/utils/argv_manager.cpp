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

#include <cstring>
#include <cassert>

#include "argv_manager.h"
#include "string_utils.h"

namespace argv {

void compose_argv_vector(std::string prog_name,
                         std::string reprompi_params,
                         const std::vector <std::string> &pgtunelib_params,
                         std::vector <std::string> *argv_vector) {
  argv_vector->push_back(prog_name);

  reprompi_params = trim(reprompi_params);
  std::vector <std::string> reprompi_params_parts = string_split(reprompi_params, ' ');
  std::copy(reprompi_params_parts.begin(), reprompi_params_parts.end(), std::back_inserter(*argv_vector));

  std::copy(pgtunelib_params.begin(), pgtunelib_params.end(), std::back_inserter(*argv_vector));
}

void convert_vector_to_argv_cstyle(const std::vector<std::string>& argv_vector,
                                   int* argc_out, char*** argv_out) {
  *argc_out = argv_vector.size();
  *argv_out = new char*[argv_vector.size()];
  for (size_t i = 0; i < argv_vector.size(); i++) {
    std::string argv_cpy_s(argv_vector.at(i));
    const size_t str_size = argv_cpy_s.size() + 1;
    (*argv_out)[i] = new char[str_size];
    snprintf((*argv_out)[i], str_size, "%s", argv_cpy_s.c_str());
    assert((*argv_out)[i][argv_cpy_s.size()] == '\0');
  }
}

void free_argv_cstyle(int argc_in, char **argv_in) {
  for (int i = 0; i < argc_in; i++) {
    delete argv_in[i];
  }
  delete argv_in;
}
}  // namespace argv


