
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

std::vector <std::string> PGData::get_columns_names() {
  return table.get_col_names();
}

std::vector<int> PGData::get_unique_counts() {
  auto counts = table.get_values_for_col_name("count");
  std::set<std::string> unique_counts(counts.begin(), counts.end());
  std::vector<std::string> ucv(unique_counts.begin(), unique_counts.end());

  std::vector<int> int_vector_res;
  for (auto iter = ucv.begin(); iter != ucv.end(); ++iter) {
    int_vector_res.push_back(fromString<int>(*iter));
  }

  return int_vector_res;
}

std::vector<double> PGData::get_runtimes_for_count(int count) {
  std::vector<double> runtimes;
  for (int rowIdx = 0; rowIdx < table.get_col_size(); rowIdx++) {
    if(fromString<int>(table.get_values_col_row("count", rowIdx)) == count) {
      runtimes.push_back(fromString<double>(table.get_values_col_row("runtime_sec", rowIdx)));
    }
  }

  return runtimes;
}