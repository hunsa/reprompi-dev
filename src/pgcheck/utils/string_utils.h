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

#ifndef SRC_PGCHECK_UTILS_STRING_UTILS_H_
#define SRC_PGCHECK_UTILS_STRING_UTILS_H_

#include <string>
#include <vector>

std::string ltrim(const std::string &s);
std::string rtrim(const std::string &s);
std::string trim(const std::string &s);
std::vector <std::string> string_split(std::string s, char delimiter);

template<typename T>
T fromString(const std::string &str) {
  std::istringstream ss(str);
  T ret;
  ss >> ret;
  return ret;
}

#endif  // SRC_PGCHECK_UTILS_STRING_UTILS_H_
