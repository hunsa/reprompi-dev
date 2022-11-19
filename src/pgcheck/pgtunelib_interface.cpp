//
// Created by Sascha on 9/22/22.
//

#include "pgtunelib_interface.h"

std::vector<std::string> split_line(std::string line);

PGTuneLibInterface::PGTuneLibInterface(std::string pgmpi_info_str)
{
  CSVFormat format;

  format.delimiter(';')
      .header_row(0);

  std::stringstream csv_stream(pgmpi_info_str);
  CSVReader csv_reader(csv_stream, format);

  for (auto& row: csv_reader) {
    std::string mpiname = row["mpiname"].get();
    if( mpi2module.count(mpiname) < 1 ) {
      mpi2module[mpiname] = std::make_tuple( row["cliname"].get(), std::vector<std::string> { row["algname"].get() } );
    } else {
      std::get<1>(mpi2module[mpiname]).push_back(row["algname"].get());
    }
  }
}

std::string PGTuneLibInterface::get_module_name_for_mpi_collectives(std::string mpi_coll_name) {
  std::string mname = "";
  if( mpi2module.count(mpi_coll_name) > 0 ) {
    mname = std::get<0>(mpi2module[mpi_coll_name]);
  }
  return mname;
}

std::vector<std::string> PGTuneLibInterface::get_available_mpi_collectives() {
  std::vector<std::string> mpi_colls;
  for(auto& e : mpi2module) {
    mpi_colls.push_back(e.first);
  }
  sort(mpi_colls.begin(), mpi_colls.end());
  return mpi_colls;
}

std::vector<std::string> PGTuneLibInterface::get_available_implementations_for_mpi_collectives(std::string mpi_coll_name) {
  std::vector<std::string> mpi_coll_versions;
  if( mpi2module.count(mpi_coll_name) > 0 ) {
    for(auto &v : std::get<1>(mpi2module[mpi_coll_name])) {
      mpi_coll_versions.push_back(v);
    }
  }
  return mpi_coll_versions;
}


