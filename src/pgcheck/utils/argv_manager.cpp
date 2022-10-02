
#include "argv_manager.h"
#include "string_utils.h"

#include <iostream>
#include <cstring>
#include <cassert>

namespace argv {

void compose_argv_vector(std::string prog_name,
                         std::string reprompi_params,
                         std::vector <std::string> &pgtunelib_params,
                         std::vector <std::string> &argv_vector) {

  // first the original program name (needed to mimic original order)
  argv_vector.push_back(prog_name);

  reprompi_params = trim(reprompi_params);
  for(std::string argv_part : string_split(reprompi_params, ' ')) {
    argv_vector.push_back(argv_part);
  }

  for(std::string argv_part : pgtunelib_params) {
    argv_vector.push_back(argv_part);
  }

}

void convert_vector_to_argv_cstyle(std::vector <std::string> &argv_vector,
                                   int *argc_out, char ***argv_out) {

  *argc_out = argv_vector.size();
  *argv_out = new char*[argv_vector.size()];
  for(size_t i=0; i<argv_vector.size(); i++) {
    std::string argv_cpy_s(argv_vector.at(i));
    //std::cout << "argv_cpy_s: " << argv_cpy_s << std::endl;
    (*argv_out)[i] = new char[argv_cpy_s.size()+1];
//    std::cout << "argv_cpy_s.size(): " << argv_cpy_s.size()
//              << " strlen(argv_cpy_s.c_str()): " << strlen(argv_cpy_s.c_str())
//              << std::endl;
    strcpy((*argv_out)[i], argv_cpy_s.c_str());
    // just making sure that we indeed end with null termination
    assert((*argv_out)[i][argv_cpy_s.size()]=='\0');
  }

}

void free_argv_cstyle(int argc_in, char **argv_in) {
  for(int i=0; i<argc_in; i++) {
    delete argv_in[i];
  }
  delete argv_in;
}

}


