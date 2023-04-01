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

#include "time_utils.h"

std::string duration_to_string(std::chrono::nanoseconds duration) {
  std::ostringstream stream;

  size_t unit_count = 0;
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);

  if (minutes.count() > 0) {
    stream << minutes.count() << " min ";
    unit_count++;
  }

  duration -= minutes;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

  if (seconds.count() > 0) {
    stream << seconds.count() << " s ";
    unit_count++;
  }

  if (unit_count > 1) { return stream.str(); }

  duration -= seconds;
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

  if (milliseconds.count() > 0) {
    stream << milliseconds.count() << " ms ";
    unit_count++;
  }

  if (unit_count > 1) { return stream.str(); }

  duration -= milliseconds;
  auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);

  if (microseconds.count() > 0) {
    stream << microseconds.count() << " µs ";
    unit_count++;
  }

  if (unit_count > 1) { return stream.str(); }

  duration -= microseconds;
  auto nanoseconds = duration;

  if (nanoseconds.count() > 0) {
    stream << nanoseconds.count() << " ns";
  }
  return stream.str();
}
