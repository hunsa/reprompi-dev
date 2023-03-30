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

#include "pgtunelib_interface.h"

std::vector <std::string> split_line(std::string line);

PGTuneLibInterface::PGTuneLibInterface(std::string pgmpi_info_str) {
  std::vector <std::string> col_names = {"cliname", "mpiname", "algname", "rooted"};
  std::stringstream csv_stream(pgmpi_info_str);
  std::string line;
  while (std::getline(csv_stream, line)) {
    std::unordered_map <std::string, std::string> row;
    int col_idx = 0;
    int line_length = line.length();
    std::string token;
    for (int idx = 0; idx < line_length; idx++) {
      // add last token string to row and reset
      if (line[idx] == ';') {
        // std::cout << "token: " << token << std::endl;
        row[col_names[col_idx++]] = token;
        token = "";
        continue;
      }
      // push char to token string
      token.push_back(line[idx]);
    }
    // add last col to row
    row[col_names[col_idx]] = token;
    if (mpi2module.count(row["mpiname"]) < 1) {
      mpi2module[row["mpiname"]] = std::make_tuple(row["cliname"], std::vector < std::string > {row["algname"]});
    } else {
      std::get<1>(mpi2module[row["mpiname"]]).push_back(row["algname"]);
    }
  }
}

std::string PGTuneLibInterface::get_module_name_for_mpi_collectives(std::string mpi_coll_name) {
  std::string mname = "";
  if (mpi2module.count(mpi_coll_name) > 0) {
    mname = std::get<0>(mpi2module[mpi_coll_name]);
  }
  return mname;
}

std::vector <std::string> PGTuneLibInterface::get_available_mpi_collectives() {
  std::vector <std::string> mpi_colls;
  for (auto &e : mpi2module) {
    mpi_colls.push_back(e.first);
  }
  sort(mpi_colls.begin(), mpi_colls.end());
  return mpi_colls;
}

std::vector <std::string>
PGTuneLibInterface::get_available_implementations_for_mpi_collectives(std::string mpi_coll_name) {
  std::vector <std::string> mpi_coll_versions;
  if (mpi2module.count(mpi_coll_name) > 0) {
    for (auto &v : std::get<1>(mpi2module[mpi_coll_name])) {
      mpi_coll_versions.push_back(v);
    }
  }
  return mpi_coll_versions;
}


