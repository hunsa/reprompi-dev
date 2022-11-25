
#ifndef REPROMPI_SRC_PGCHECK_UTILS_CSV_PARSER_H
#define REPROMPI_SRC_PGCHECK_UTILS_CSV_PARSER_H

#include <vector>
#include <unordered_map>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <fstream>
#include <set>
#include "csv_parser.h"
#include "../pgdata_table.h"
#include "../pgdata.h"
#include "../comparer/comparer_data.h"

class CSVParser {

public:

  CSVParser() = default;
  PGDataTable parse_file(std::string csv_path);

};

#endif //REPROMPI_SRC_PGCHECK_UTILS_CSV_PARSER_H
