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

#include "pgcheck_helper.h"

/**
 * not my code
 * https://raymii.org/s/articles/Execute_a_command_and_get_both_output_and_exit_code.html
 * @param cmd
 * @return
 */

std::string exec_command(const std::string &command) {
  std::array<char, 1048576> buffer {};
  std::string result;

  FILE *pipe = popen(command.c_str(), "r");
  if (pipe == nullptr) {
    throw std::runtime_error("popen() failed!");
  }
  try {
    std::size_t bytesread;
    while ((bytesread = std::fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0) {
      result += std::string(buffer.data(), bytesread);
    }
  } catch (...) {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return result;
}

