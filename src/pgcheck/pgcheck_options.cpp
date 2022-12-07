//
// Created by Max on 11/16/22.
//

#include "pgcheck_options.h"

bool PGCheckOptions::get_merge_coll_tables() {
  return merge_coll_tables;
}

bool PGCheckOptions::get_print_to_csv() {
  return csv;
}

bool PGCheckOptions::get_verbose() {
  return verbose;
}

bool PGCheckOptions::get_csv() {
  return csv;
}

int PGCheckOptions::get_comparer_type() {
  return comparer_type;
}

std::string PGCheckOptions::get_input_file() {
  return input_file;
}

std::string PGCheckOptions::get_output_directory() {
  return output_directory;
}

std::string PGCheckOptions::get_config_message() {
  return config_message;
}

int PGCheckOptions::parse(int argc, char *argv[]) {
  int c;
  struct option long_opts[] =
      {
          {"help",     no_argument,       NULL, 'h'},
          {"merge",    no_argument,       NULL, 'm'},
          {"csv",      no_argument,       NULL, 's'},
          {"verbose",  no_argument,       NULL, 'v'},
          {"input",    required_argument, NULL, 'f'},
          {"output",   required_argument, NULL, 'o'},
          {"comparer", required_argument, NULL, 'c'},
          {NULL,       0,                 NULL, 0  }
      };

  while ((c = getopt_long(argc, argv, ":hmsvf:o:c:", long_opts, NULL)) != -1) {
    switch (c) {
      case 'h':
      case '?':
      default :
        return -1;
      case 'f':
        input_file = std::string(optarg);
        break;
      case 'c':
        comparer_type = std::atoi(optarg);
        break;
      case 'o':
        output_directory = std::string(optarg);
        break;
      case 'm':
        merge_coll_tables = true;
        break;
      case 's':
        csv = true;
        break;
      case 'v':
        verbose = true;
        break;
    }
  }

  optind = 1;
  opterr = 1;

  // print results to cout if output directory is not present
  struct stat sb;
  if((stat(output_directory.c_str(), &sb)!=0) && !output_directory.empty()) {
    std::string not_found_directory = output_directory;
    output_directory = "";
    verbose = true;

    if(csv) {
      csv = false;
      std::cout << "\033[35m" << "Warning: " << "cannot find directory '" << not_found_directory << "' -> option verbose was enabled, option csv was disabled" << "\033[0m" << std::endl;
    }
    std::cout << "\033[35m" << "Warning: " << "cannot find directory '" << not_found_directory << "' -> option verbose was enabled" << "\033[0m" << std::endl;
  }

  // print results to cout if output directory was not specified
  if(output_directory.empty() && !verbose) {
    verbose = true;
    std::cout << "\033[35m" << "Warning: " << "output directory was not specified -> option verbose was enabled and output is written to cout" << "\033[0m" << std::endl;
  }

  if(!output_directory.empty() && (stat(output_directory.c_str(), &sb)==0)) {
    std::string slash = "/";
    if (!std::equal(slash.rbegin(), slash.rend(), output_directory.rbegin())) {
      output_directory = output_directory.append("/");
    }
  }

  return 1;
}
