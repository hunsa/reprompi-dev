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
                           std::vector <std::string> &pgtunelib_params,
                           std::vector <std::string> &argv_vector) {
    // first the original program name (needed to mimic original order)
    argv_vector.push_back(prog_name);

    reprompi_params = trim(reprompi_params);
    for (std::string argv_part : string_split(reprompi_params, ' ')) {
      argv_vector.push_back(argv_part);
    }

    for (std::string argv_part : pgtunelib_params) {
      argv_vector.push_back(argv_part);
    }
  }

  void convert_vector_to_argv_cstyle(std::vector <std::string> &argv_vector,
                                     int *argc_out, char ***argv_out) {
    *argc_out = argv_vector.size();
    *argv_out = new char *[argv_vector.size()];
    for (size_t i = 0; i < argv_vector.size(); i++) {
      std::string argv_cpy_s(argv_vector.at(i));
      // std::cout << "argv_cpy_s: " << argv_cpy_s << std::endl;
      (*argv_out)[i] = new char[argv_cpy_s.size() + 1];
      // std::cout << "argv_cpy_s.size(): " << argv_cpy_s.size()
      // << " strlen(argv_cpy_s.c_str()): " << strlen(argv_cpy_s.c_str())
      // << std::endl;
      strcpy((*argv_out)[i], argv_cpy_s.c_str());
      // just making sure that we indeed end with null termination
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


