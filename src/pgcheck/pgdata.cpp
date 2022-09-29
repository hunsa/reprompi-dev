
#include <sstream>
#include <fstream>
#include <set>
#include "pgdata.h"

PGData::PGData(std::string mpi_coll_name, std::string mockup_name) :
  mpi_coll_name(mpi_coll_name),
  mockup_name(mockup_name)
{
}

int PGData::read_csv_from_file(std::string csv_path) {

  std::ifstream infile(csv_path);
  std::string line;
  std::string out = "";

  while (std::getline(infile, line))
  {
    if( line[0] != '#' ) {

      std::istringstream iss(line);
      std::string token;
      bool first = true;

      while (iss >> token) {
        //std::cout << "token: " << token << std::endl;
        if( ! first ) {
          out.append(";");
        } else {
          first = false;
        }
        out.append(token);
      }
      out.append("\n");
    }
  }

  //std::cout << "out: " << out << std::endl;
  std::stringstream sstream(out);

  csv = rapidcsv::Document(sstream,
                           rapidcsv::LabelParams(),
                           rapidcsv::SeparatorParams(';', true),
                           rapidcsv::ConverterParams(),
                           rapidcsv::LineReaderParams(true,'#'));

  //std::cout << "col num: " << csv.GetColumnCount() << std::endl;

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
  //std::cout << "counts.size: " << counts.size() << std::endl;
  std::set<int> unique_counts(counts.begin(), counts.end());
  std::vector<int> ucv(unique_counts.begin(), unique_counts.end());
  return ucv;
}