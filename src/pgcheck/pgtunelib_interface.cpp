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

//#include "csv.hpp"
//using namespace csv;


std::vector<std::string> split_line(std::string line);

PGTuneLibInterface::PGTuneLibInterface(std::string pgmpi_info_str)
{
//  CSVFormat format;
//  std::stringstream csv_stream(pgmpi_info_str);
//  CSVReader csv_reader(csv_stream, format);

  std::stringstream csv_stream(pgmpi_info_str);
  std::string line;

  // read header
  std::getline(csv_stream, line);
  auto tokens = split_line(line);

  while( std::getline(csv_stream, line) ) {
    std::cout << "LINE:" << line << std::endl;
  }

}


std::vector<std::string> split_line(std::string line) {
  std::vector <std::string> tokens;
  std::string tok;
  auto line_stream = std::stringstream(line);
  while(getline(line_stream, tok, ';'))
  {
    tokens.push_back(tok);
    std::cout << tok << std::endl;
  }
  return tokens;
}


