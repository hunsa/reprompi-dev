//
// Created by Sascha on 9/22/22.
//

#include "pgtunelib_interface.h"
#include "csv.hpp"

using namespace csv;

PGTuneLibInterface::PGTuneLibInterface(std::string pgmpi_info_str)
{
  CSVFormat format;
  std::stringstream csv_stream(pgmpi_info_str);
  CSVReader csv_reader(csv_stream, format);



}
