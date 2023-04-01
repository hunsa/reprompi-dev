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

#include "logger.h"

namespace Logger {
Level Logger::verbose_level_ = Level::DEFAULT;

bool Logger::is_root() {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  return my_rank == 0;
}

void Logger::log(const std::string &message) {
  if (is_root()) {
    std::cout << message;
  }
}

void Logger::info(const std::string &message) {
  if (is_root()) {
    std::cout << "\033[34m" << "[INFO]    " << "\033[0m" << message << std::endl;
  }
}

void Logger::info_verbose(const std::string &message) {
  if (is_root() && Logger::verbose_level_ == Level::VERBOSE) {
    std::cout << "\033[34m" << "[INFO]    " << "\033[0m" << message << std::endl;
  }
}

void Logger::warn(const std::string &message) {
  if (is_root()) {
    std::cout << "\033[35m" << "[WARNING] " << "\033[0m" << message << std::endl;
  }
}

void Logger::error(const std::string &message) {
  if (is_root()) {

    std::cerr << "\033[31m" << "[ERROR]   " << "\033[0m" << message << std::endl;
  }
}

void Logger::evaluation_block(const std::string &heading, const std::string &message) {
  if (is_root()) {
    std::cout << std::endl << "\033[33m" << "[EVALUATION | " << heading << "]       " << "\033[0m" << std::endl;
    std::cout << message;
    std::cout << "\033[33m" << "[EVALUATION | " << heading << "] " << "\033[0m" << std::endl << std::endl;
  }
}

void Logger::write_file(const std::string &message, const std::string &filename) {
  std::ofstream file(filename);
  file << message << std::endl;
  file.close();
}
}  // namespace Logger
