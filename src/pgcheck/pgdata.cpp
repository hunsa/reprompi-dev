
#include "pgdata.h"

PGData::PGData(std::string csv_string) {
  csv = rapidcsv::Document(csv_string,
                           rapidcsv::LabelParams(),
                           rapidcsv::SeparatorParams(),
                           rapidcsv::ConverterParams(),
                           rapidcsv::LineReaderParams(true,'#'));
}

std::vector<std::string> PGData::get_columns_names() {
  return csv.GetColumnNames();
}