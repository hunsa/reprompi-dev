//
// Created by Sascha on 9/22/22.
//

#include "pgtunelib_interface.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <istream>

#include "csv.hpp"
using namespace csv;


std::vector<std::string> split_line(std::string line);

PGTuneLibInterface::PGTuneLibInterface(std::string pgmpi_info_str)
{
  CSVFormat format;
  std::stringstream csv_stream(pgmpi_info_str);
  CSVReader csv_reader(csv_stream, format);
  
}


