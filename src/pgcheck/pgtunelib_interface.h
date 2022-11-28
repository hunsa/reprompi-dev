//
// Created by Sascha on 9/22/22.
//

#ifndef REPROMPI_SRC_PGCHECK_PGTUNELIB_INTERFACE_H
#define REPROMPI_SRC_PGCHECK_PGTUNELIB_INTERFACE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

class PGTuneLibInterface {

private:
  std::vector <std::string> mpi_collectives;
  std::unordered_map <std::string, std::tuple<std::string, std::vector<std::string>>>
  mpi2module;

public:
  PGTuneLibInterface(std::string pgmpi_info_str);

  std::vector <std::string> get_available_mpi_collectives();

  std::string get_module_name_for_mpi_collectives(std::string mpi_coll_name);

  std::vector <std::string> get_available_implementations_for_mpi_collectives(std::string mpi_coll_name);

};

#endif //REPROMPI_SRC_PGCHECK_PGTUNELIB_INTERFACE_H
