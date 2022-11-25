
#include "csv_parser.h"

PGDataTable CSVParser::parse_file(std::string csv_path) {
  PGDataTable table_res{};

  std::ifstream infile(csv_path);
  std::string line;

  std::vector <std::string> col_names = {"test", "nrep", "count", "runtime_sec"};
  table_res.set_col_names(col_names);
  bool first = true;

  while (std::getline(infile, line)) {

    std::unordered_map <std::string, std::string> row;

    if (line[0] != '#') {

      if (first) {
        first = false;
        std::getline(infile, line);
      }

      std::istringstream iss(line);
      std::string token;
      int col_idx = 0;

      while (iss >> token) {
        //std::cout << "token: " << token << std::endl;
        row[col_names[col_idx++]] = token;
      }
    }

    table_res.add_row(row);
  }

  return table_res;
}