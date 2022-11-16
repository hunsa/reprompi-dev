//
// Created by Max on 11/16/22.
//

#include "pgcheck_options.h"

PGCheckOptions::PGCheckOptions(int argc, char *argv[]) {
  int c;

  const char* short_opts = "hf:c:o:dsv";
  struct option long_opts[] =
  {
      {"help",     no_argument,       NULL, 'h'},
      {"input",    required_argument, NULL, 'f'},
      {"output",   required_argument, NULL, 'o'},
      {"comparer", required_argument, NULL, 'c'},
      {"detailed", no_argument, NULL, 'd'},
      {"csv",      no_argument, NULL, 's'},
      {"verbose",      no_argument, NULL, 'v'},
      {NULL,       0,                 NULL, 0  }
  };

  while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
    switch (c) {
      case '?':
      case 'h':
      default :
        print_usage(argv[0]);
        exit(-1);
      case 'f':
        input_file = std::string(optarg);
        break;
      case 'c':
        comparer_type = std::atoi(optarg);
        break;
      case 'o':
        output_directory = std::string(optarg);
        break;
      case 'd':
        detailed = true;
        break;
      case 's':
        csv = true;
        break;
      case 'v':
        verbose = true;
        break;
    }
  }

  struct stat sb;
  if(stat(output_directory.c_str(), &sb)!=0) {
    std::cerr << "Cannot find " << output_directory << ". Write results to stdout." << std::endl;
    output_directory = "";
  }

}

void PGCheckOptions::print_usage(char *command) {
  std::cout << "USAGE: " << std::string(command) << " -f input_file [options]" << std::endl << std::endl;
  std::cout << "OPTIONS:" << std::endl;
  std::cout << std::setw(28) << std::left << "  ?, -h, --help" << "Display this information." << std::endl;
  std::cout << std::setw(28) << std::left << "  -c, --comparer {0|1|2|3}" << "Specify the comparer type (0=Simple; 1=Detailed; 2=T-Test; 3=Grouped T-Test)." << std::endl;
  std::cout << std::setw(28) << std::left << "  -o, --output <path>" << "Specify an existing output folder." << std::endl;
  std::cout << std::setw(28) << std::left << "  -d, --detailed" << "Print detailed output." << std::endl;
  std::cout << std::setw(28) << std::left << "  -s, --csv" << "Print results to .csv file." << std::endl;
  std::cout << std::setw(28) << std::left << "  -v, --verbose" << "Print additional information." << std::endl;
}

std::string PGCheckOptions::get_input_file() {
  return input_file;
}

std::string PGCheckOptions::get_output_directory() {
  return output_directory;
}

bool PGCheckOptions::get_print_detailed_output() {
  return detailed;
}

bool PGCheckOptions::get_print_to_csv() {
  return csv;
}

int PGCheckOptions::get_comparer_type() {
  return comparer_type;
}

bool PGCheckOptions::get_verbose() {
  return verbose;
}

