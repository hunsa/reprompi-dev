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

#include "pgdata.h"

PGData::PGData(std::string mpi_coll_name, std::string mockup_name) :
    mpi_coll_name(mpi_coll_name),
    mockup_name(mockup_name) {
}

int PGData::read_csv_from_file(std::string csv_path) {
  CSVParser *parser = new CSVParser();
  table = parser->parse_file(csv_path);
  return 0;
}

std::string PGData::get_mockup_name() {
  return mockup_name;
}

std::vector <std::string> PGData::get_columns_names() {
  return table.get_column_names();
}

std::vector<int> PGData::get_unique_counts() {
  auto counts = table.get_values_for_col_name("count");
  std::set <std::string> unique_counts(counts.begin(), counts.end());
  std::vector <std::string> ucv(unique_counts.begin(), unique_counts.end());

  std::vector<int> int_vector_res;
  for (auto iter = ucv.begin(); iter != ucv.end(); ++iter) {
    int_vector_res.push_back(fromString<int>(*iter));
  }
  sort(int_vector_res.begin(), int_vector_res.end());
  return int_vector_res;
}

std::vector<double> PGData::get_runtimes_for_count(int count) {
  std::vector<double> runtimes;
  for (int rowIdx = 0; rowIdx < table.get_col_size(); rowIdx++) {
    if (fromString<int>(table.get_values_col_row("count", rowIdx)) == count) {
      runtimes.push_back(fromString<double>(table.get_values_col_row("runtime_sec", rowIdx)));
    }
  }

  return runtimes;
}
