
#include <set>
#include "pgdata.h"

PGData::PGData(std::string mpi_coll_name, std::string mockup_name) :
  mpi_coll_name(mpi_coll_name),
  mockup_name(mockup_name)
{
}

int PGData::read_csv_from_file(std::string csv_path) {
  csv = rapidcsv::Document(csv_path,
                           rapidcsv::LabelParams(),
                           rapidcsv::SeparatorParams(),
                           rapidcsv::ConverterParams(),
                           rapidcsv::LineReaderParams(true,'#'));
  return 0;
}

std::vector<std::string> PGData::get_columns_names() {
  return csv.GetColumnNames();
}

std::vector<double> PGData::get_runtimes_for_count(int count) {
  auto rt = csv.GetColumn<double>("runtime_sec");
  return rt;
}

std::vector<int> PGData::get_unique_counts() {
  auto counts = csv.GetColumn<int>("count");
  std::cout << "counts.size: " << counts.size() << std::endl;
  std::set<int> unique_counts(counts.begin(), counts.end());
  std::vector<int> ucv(unique_counts.begin(), unique_counts.end());
  return ucv;
}