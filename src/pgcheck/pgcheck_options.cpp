/*  PGChecker - MPI Performance Guidelines Checker
 *
 *  Copyright 2023 Sascha Hunold, Maximilian Hagn
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#include "pgcheck_options.h"
#include <filesystem>
namespace fs = std::filesystem;

bool PGCheckOptions::get_merge_coll_tables() {
  return merge_coll_tables;
}

bool PGCheckOptions::get_print_to_csv() {
  return csv;
}

bool PGCheckOptions::is_verbose() {
  return verbose;
}

//bool PGCheckOptions::get_allow_mkdir() {
//  return allow_mkdir;
//}

bool PGCheckOptions::get_csv() {
  return csv;
}

int PGCheckOptions::get_test_type() {
  return test_type;
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

std::vector<int> PGCheckOptions::get_comparer_list() {
  return comparer_list;
}

bool PGCheckOptions::parse(int argc, char *argv[], bool is_root) {
  int c;

  struct option long_opts[] = {
          {"help",        no_argument,       NULL, 'h'},
          {"merge",       no_argument,       NULL, 'm'},
          {"csv",         no_argument,       NULL, 's'},
          {"verbose",     no_argument,       NULL, 'v'},
          {"allow-mkdir", no_argument,       NULL, 'd'},
          {"input",       required_argument, NULL, 'f'},
          {"output",      required_argument, NULL, 'o'},
          {"test",        required_argument, NULL, 't'},
          {"comp-list",   required_argument, NULL, 'c'},
          {NULL,          0,                 NULL, 0}
      };

  while ((c = getopt_long(argc, argv, ":hdmsvf:o:c:t:", long_opts, NULL)) != -1) {
    switch (c) {
      case 'h':
      case '?':
      default :
        return -1;
      case 'f':
        input_file = std::string(optarg);
        break;
      case 't':
        test_type = std::atoi(optarg);
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
      case 'd':
        allow_mkdir = true;
        break;
      case 'c':
        std::string comp_string = optarg;
        std::string delimiter = ",";
        size_t pos = 0;
        std::string token;

        while ((pos = comp_string.find(delimiter)) != std::string::npos) {
          token = comp_string.substr(0, pos);
          comparer_list.push_back(std::stoi(token));
          comp_string.erase(0, pos + delimiter.length());
        }

        if (!comp_string.empty()) {
          comparer_list.push_back(std::stoi(comp_string));
        }

        break;
    }
  }

  optind = 1;
  opterr = 1;

  // comparer 5 is default
  if (comparer_list.size() < 1) {
    comparer_list.push_back(5);
  }

  // unique and sorted comparer list if list has multiple entries
  if (comparer_list.size() > 1) {
    comparer_list.erase(std::unique(comparer_list.begin(), comparer_list.end()), comparer_list.end());
    std::sort(comparer_list.begin(), comparer_list.end());
  }

  if( is_root ) {
    if(output_directory.empty()) {
      std::cerr << "\033[35m" << "Error: " << "no output directory given, use -o" << "\033[0m" << std::endl;
      return -1;
    }

    fs::path out{output_directory};
    if (!fs::is_directory(out)) {
      if (allow_mkdir) {
        bool okay = fs::create_directory(out);
        if (!okay) {
          std::cerr << "\033[35m" << "Error: " << "cannot create directory '" << output_directory << "\033[0m"
                    << std::endl;
        }
      } else {
        std::cerr << "\033[35m" << "Error: " << "directory '" << output_directory
                  << "' does not exists. (use -d to force creation of output dir)" << "\033[0m" << std::endl;
        return -1;
      }
    }
  }


//  // print results to cout if output directory was not specified
//  if (output_directory.empty() && !verbose) {
//    verbose = true;
//    std::cout << "\033[35m" << "Warning: "
//              << "output directory was not specified -> option verbose was enabled and output is written to cout"
//              << "\033[0m" << std::endl;
//  }
//
//  if (!output_directory.empty() && (stat(output_directory.c_str(), &sb) == 0)) {
//    std::string slash = "/";
//    if (!std::equal(slash.rbegin(), slash.rend(), output_directory.rbegin())) {
//      output_directory = output_directory.append("/");
//    }
//  }

  return 0;
}
