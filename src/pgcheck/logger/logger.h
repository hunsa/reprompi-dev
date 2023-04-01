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

#ifndef SRC_PGCHECK_LOGGER_LOGGER_H_
#define SRC_PGCHECK_LOGGER_LOGGER_H_

#include <string>
#include <ostream>
#include <iostream>

#include "mpi.h"

namespace Logger {
enum Level {
  DEFAULT, VERBOSE
};

class Logger {
 public:
  static Logger& instance() {
    static Logger logger;
    return logger;
  }

  static void set_verbose_level(Level level) {
    verbose_level_ = level;
  }

  void log(const std::string& message);
  void info(const std::string& message);
  void info_verbose(const std::string& message);
  void warn(const std::string& message);
  void error(const std::string& message);
  void evaluation_block(const std::string& header, const std::string& message);

 private:
  Logger() {}
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  bool is_root();

  static Level verbose_level_;
};
}  // namespace Logger

#define LOGGER Logger::Logger::instance()
#define LOG(message) LOGGER.log(message)
#define INFO(message) LOGGER.info(message)
#define INFO_VERBOSE(message) LOGGER.info_verbose(message)
#define WARN(message) LOGGER.warn(message)
#define ERROR(message) LOGGER.error(message)
#define EVAL_BLOCK(header, message) LOGGER.evaluation_block(header, message)

#endif  // SRC_PGCHECK_LOGGER_LOGGER_H_
